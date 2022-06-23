#pragma once

struct Cancellable {
	bool cancelled = false;

	void cancel() {
		cancelled = true;
	}

	operator bool() {
		return !cancelled;
	}
};