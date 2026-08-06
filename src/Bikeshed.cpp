#include "Bikeshed.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;
	p->addModel(breakpointModel);
	p->addModel(entropyPoolModel);
	p->addModel(entropyPuddleModel);
}
