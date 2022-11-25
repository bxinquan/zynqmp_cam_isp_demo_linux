/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * thread.h - Thread support
 */

#pragma once

#include <memory>
#include <sys/types.h>
#include <thread>

#include <libcamera/base/private.h>

#include <libcamera/base/message.h>
#include <libcamera/base/signal.h>
#include <libcamera/base/utils.h>

namespace libcamera {

class EventDispatcher;
class Message;
class Object;
class ThreadData;
class ThreadMain;

class Thread
{
public:
	Thread();
	virtual ~Thread();

	void start();
	void exit(int code = 0);
	bool wait(utils::duration duration = utils::duration::max());

	bool isRunning();

	Signal<> finished;

	static Thread *current();
	static pid_t currentId();

	EventDispatcher *eventDispatcher();

	void dispatchMessages(Message::Type type = Message::Type::None);

protected:
	int exec();
	virtual void run();

private:
	void startThread();
	void finishThread();

	void postMessage(std::unique_ptr<Message> msg, Object *receiver);
	void removeMessages(Object *receiver);

	friend class Object;
	friend class ThreadData;
	friend class ThreadMain;

	void moveObject(Object *object);
	void moveObject(Object *object, ThreadData *currentData,
			ThreadData *targetData);

	std::thread thread_;
	ThreadData *data_;
};

} /* namespace libcamera */
