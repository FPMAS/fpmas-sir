#ifndef MACROPOP_CONFIG_H
#define MACROPOP_CONFIG_H

namespace macropop {
	enum GraphMode {
		CLUSTERED,
		UNIFORM
	};

	enum SyncMode {
		GHOST,
		HARD_SYNC
	};

	enum LbMethod {
		ZOLTAN,
		RANDOM
	};
}
#endif
