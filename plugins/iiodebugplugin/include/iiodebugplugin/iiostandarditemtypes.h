#ifndef SCOPY_IIOSTANDARDITEMTYPES_H
#define SCOPY_IIOSTANDARDITEMTYPES_H

namespace scopy::iiodebugplugin {
enum IIOStandardItemType
{
	Undefined = 1000,
	Context = 1001,
	Device = 1002,
	Channel = 1003,
	ContextAttribute = 1004,
	DeviceAttribute = 1005,
	ChannelAttribute = 1006,
};
}

#endif // SCOPY_IIOSTANDARDITEMTYPES_H
