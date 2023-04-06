// SPDX-License-Identifier: GPL-2.0
/*
 * Intel Platform Monitory Technology Telemetry driver
 *
 * Copyright (c) 2020, Intel Corporation.
 * All Rights Reserved.
 *
 * Author: "David E. Box" <david.e.box@linux.intel.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/overflow.h>

#include "intel_pmt_class.h"
#include "intel_pmt_telemetry.h"
#include "version/module_version.h"

#define TELEM_DEV_NAME		"pmt_telemetry"

#define TELEM_SIZE_OFFSET	0x0
#define TELEM_GUID_OFFSET	0x4
#define TELEM_BASE_OFFSET	0x8
#define TELEM_ACCESS(v)		((v) & GENMASK(3, 0))
/* size is in bytes */
#define TELEM_SIZE(v)		(((v) & GENMASK(27, 12)) >> 10)

/* Used by client hardware to identify a fixed telemetry entry*/
#define TELEM_CLIENT_FIXED_BLOCK_GUID	0x10000000

#define NUM_BYTES_QWORD(v)	((v) << 3)
#define SAMPLE_ID_OFFSET(v)	((v) << 3)

static DEFINE_MUTEX(list_lock);
static BLOCKING_NOTIFIER_HEAD(telem_notifier);

struct pmt_telem_priv {
	int				num_entries;
	struct intel_pmt_entry		entry[];
};

static bool pmt_telem_region_overlaps(struct intel_pmt_entry *entry,
				      struct device *dev)
{
	u32 guid = readl(entry->disc_table + TELEM_GUID_OFFSET);

	if (guid != TELEM_CLIENT_FIXED_BLOCK_GUID)
		return false;

	return intel_pmt_is_early_client_hw(dev);
}

static int pmt_telem_header_decode(struct intel_pmt_entry *entry,
				   struct intel_pmt_header *header,
				   struct device *dev,
				   struct resource *disc_res)
{
	void __iomem *disc_table = entry->disc_table;

	if (pmt_telem_region_overlaps(entry, dev))
		return 1;

	header->access_type = TELEM_ACCESS(readl(disc_table));
	header->guid = readl(disc_table + TELEM_GUID_OFFSET);
	header->base_offset = readl(disc_table + TELEM_BASE_OFFSET);

	/* Size is measured in DWORDS, but accessor returns bytes */
	header->size = TELEM_SIZE(readl(disc_table));

	entry->header.access_type = header->access_type;
	entry->header.guid = header->guid;
	entry->header.base_offset = header->base_offset;
	entry->header.size = header->size;

	return 0;
}

static DEFINE_XARRAY_ALLOC(telem_array);
static struct intel_pmt_namespace pmt_telem_ns = {
	.name = "telem",
	.xa = &telem_array,
	.pmt_header_decode = pmt_telem_header_decode,
};

/* Called when all users unregister and the device is removed */
static void pmt_telem_ep_release(struct kref *kref)
{
	struct telem_endpoint *ep;

	pr_debug("%s: begin release kref\n", __func__);
	ep = container_of(kref, struct telem_endpoint, kref);
	kfree(ep);
	pr_debug("%s: end release kref for %px\n", __func__, ep);
}

/*
 * driver api
 */
int pmt_telem_get_next_endpoint(int start)
{
	struct intel_pmt_entry *entry;
	unsigned long found_idx;

	mutex_lock(&list_lock);
	xa_for_each_start(&telem_array, found_idx, entry, start) {
		/*
		 * Return first found index after start.
		 * 0 is not valid id.
		 */
		if (found_idx > start)
			break;
	}
	mutex_unlock(&list_lock);

	return found_idx == start ? 0 : found_idx;
}
EXPORT_SYMBOL_GPL(pmt_telem_get_next_endpoint);

struct telem_endpoint *pmt_telem_register_endpoint(int devid)
{
	struct intel_pmt_entry *entry;
	unsigned long index = devid;

	mutex_lock(&list_lock);
	entry = xa_find(&telem_array, &index, index, XA_PRESENT);
	if (!entry) {
		mutex_unlock(&list_lock);
		return ERR_PTR(-ENXIO);
	}

	kref_get(&entry->ep->kref);

	pr_debug("%s: kref for [%px] is now %d\n", __func__, entry, kref_read(&entry->ep->kref));
	mutex_unlock(&list_lock);

	return entry->ep;
}
EXPORT_SYMBOL_GPL(pmt_telem_register_endpoint);

void pmt_telem_unregister_endpoint(struct telem_endpoint *ep)
{
	kref_put(&ep->kref, pmt_telem_ep_release);
	pr_debug("%s: kref for [%px] is now %d\n", __func__, ep, kref_read(&ep->kref));
}
EXPORT_SYMBOL(pmt_telem_unregister_endpoint);

int pmt_telem_get_endpoint_info(int devid,
				struct telem_endpoint_info *info)
{
	struct intel_pmt_entry *entry;
	unsigned long index = devid;
	int err = 0;

	if (!info)
		return -EINVAL;

	mutex_lock(&list_lock);
	entry = xa_find(&telem_array, &index, index, XA_PRESENT);
	if (!entry) {
		err = -ENXIO;
		goto unlock;
	}

	info->pdev = entry->ep->parent;
	info->header = entry->ep->header;

unlock:
	mutex_unlock(&list_lock);
	return err;

}
EXPORT_SYMBOL_GPL(pmt_telem_get_endpoint_info);

int
pmt_telem_read(struct telem_endpoint *ep, u32 id, u64 *data, u32 count)
{
	u32 offset, size;

	if (!ep->present)
		return -ENODEV;

	offset = SAMPLE_ID_OFFSET(id);
	size = ep->header.size;

	if ((offset + NUM_BYTES_QWORD(count)) > size)
		return -EINVAL;

	pr_debug("%s: Reading id %d, offset 0x%x, count %d, base %px\n",
	       __func__, id, SAMPLE_ID_OFFSET(id), count, ep->base);
	memcpy_fromio(data, ep->base + offset, NUM_BYTES_QWORD(count));

	return ep->present ? 0 : -EPIPE;
}
EXPORT_SYMBOL_GPL(pmt_telem_read);

int pmt_telem_register_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&telem_notifier, nb);
}
EXPORT_SYMBOL(pmt_telem_register_notifier);

int pmt_telem_unregister_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&telem_notifier, nb);
}
EXPORT_SYMBOL(pmt_telem_unregister_notifier);

static int pmt_telem_add_endpoint(struct device *dev,
				  struct pmt_telem_priv *priv,
				  struct intel_pmt_entry *entry)
{
	struct telem_endpoint *ep;

	/*
	 * Endpoint lifetimes are managed by kref, not devres.
	 */
	entry->ep = kzalloc(sizeof(*(entry->ep)), GFP_KERNEL);
	if (!entry->ep)
		return -ENOMEM;

	ep = entry->ep;
	ep->parent = to_pci_dev(dev->parent);
	ep->header.access_type = entry->header.access_type;
	ep->header.guid = entry->header.guid;
	ep->header.base_offset = entry->header.base_offset;
	ep->header.size = entry->header.size;

	/* use the already ioremapped entry base */
	ep->base = entry->base;
	ep->present = true;

	kref_init(&ep->kref);

	return 0;
}

static int pmt_telem_remove(struct platform_device *pdev)
{
	struct pmt_telem_priv *priv = platform_get_drvdata(pdev);
	struct intel_pmt_entry *entry;
	int i;

	dev_dbg(&pdev->dev, "%s\n", __func__);

	for (i = 0, entry = priv->entry; i < priv->num_entries; i++, entry++) {
		blocking_notifier_call_chain(&telem_notifier,
					     PMT_TELEM_NOTIFY_REMOVE,
					     &entry->devid);
		kref_put(&priv->entry[i].ep->kref, pmt_telem_ep_release);
		dev_dbg(&pdev->dev, "kref count of ep #%d [%px] is %d\n", i, entry->ep, kref_read(&entry->ep->kref));
		intel_pmt_dev_destroy(&priv->entry[i], &pmt_telem_ns);
	}

	return 0;
}

static int pmt_telem_probe(struct platform_device *pdev)
{
	struct intel_pmt_entry *entry;
	struct pmt_telem_priv *priv;
	size_t size;
	int i, ret;

	size = struct_size(priv, entry, pdev->num_resources);
	priv = devm_kzalloc(&pdev->dev, size, GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	for (i = 0; i < pdev->num_resources; i++) {
		struct intel_pmt_entry *entry = &priv->entry[i];

		dev_dbg(&pdev->dev, "Getting resource %d\n", i);
		ret = intel_pmt_dev_create(entry, &pmt_telem_ns, pdev, i);
		if (ret < 0)
			goto abort_probe;
		if (ret)
			continue;

		priv->num_entries++;

		ret = pmt_telem_add_endpoint(&pdev->dev, priv, entry);
		if (ret)
			goto abort_probe;

		dev_dbg(&pdev->dev, "kref count of ep #%d [%px] is %d\n", i, entry->ep, kref_read(&entry->ep->kref));
	}

	for (i = 0, entry = priv->entry; i < priv->num_entries; i++, entry++)
		blocking_notifier_call_chain(&telem_notifier,
					     PMT_TELEM_NOTIFY_ADD,
					     &entry->devid);

	return 0;

abort_probe:
	pmt_telem_remove(pdev);
	return ret;
}

static struct platform_driver pmt_telem_driver = {
	.driver = {
		.name   = TELEM_DEV_NAME,
	},
	.remove = pmt_telem_remove,
	.probe  = pmt_telem_probe,
};

static int __init pmt_telem_init(void)
{
	return platform_driver_register(&pmt_telem_driver);
}
module_init(pmt_telem_init);

static void __exit pmt_telem_exit(void)
{
	platform_driver_unregister(&pmt_telem_driver);
	xa_destroy(&telem_array);
}
module_exit(pmt_telem_exit);

MODULE_AUTHOR("David E. Box <david.e.box@linux.intel.com>");
MODULE_DESCRIPTION("Intel PMT Telemetry driver");
MODULE_ALIAS("platform:" TELEM_DEV_NAME);
MODULE_LICENSE("GPL v2");
MODULE_VERSION(BACKPORT_MODULE_VER);
