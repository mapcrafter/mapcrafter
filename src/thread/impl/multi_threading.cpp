/*
 * multi_threading.cpp
 *
 *  Created on: 30.12.2013
 *      Author: moritz
 */

#include "multi_threading.h"

namespace mapcrafter {
namespace thread {

MultiThreadingDispatcher::MultiThreadingDispatcher() {
}

MultiThreadingDispatcher::~MultiThreadingDispatcher() {
}

void MultiThreadingDispatcher::dispatch(const RenderWorkContext& context,
		std::shared_ptr<util::IProgressHandler> progress) {
}

} /* namespace thread */
} /* namespace mapcrafter */
