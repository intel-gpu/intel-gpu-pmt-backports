#include <linux/auxiliary_bus.h>
#include <linux/version.h>
#include <osv_version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,17,0) && !SUSE_RELEASE_VERSION_GEQ(15,4,0)
static inline void *auxiliary_get_drvdata(struct auxiliary_device *auxdev)
{
	return dev_get_drvdata(&auxdev->dev);
}

static inline void auxiliary_set_drvdata(struct auxiliary_device *auxdev, void *data)
{
	dev_set_drvdata(&auxdev->dev, data);
}
#endif