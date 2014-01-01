/*
 * multi_threading.h
 *
 *  Created on: 30.12.2013
 *      Author: moritz
 */

#ifndef MULTI_THREADING_H_
#define MULTI_THREADING_H_

#include "../dispatcher.h"

namespace mapcrafter {
namespace thread {

class MultiThreadingDispatcher : public Dispatcher {
public:
	MultiThreadingDispatcher();
	virtual ~MultiThreadingDispatcher();

	virtual void dispatch(const RenderWorkContext& context,
			std::shared_ptr<util::IProgressHandler> progress);
};

} /* namespace thread */
} /* namespace mapcrafter */

#endif /* MULTI_THREADING_H_ */
