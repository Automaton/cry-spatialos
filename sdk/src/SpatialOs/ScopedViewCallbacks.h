#pragma once
#include "improbable/worker.h"

/**
* Automatically disconnect callbacks from a View upon destruction of the
* object.
*/
class ScopedViewCallbacks
{
public:
	ScopedViewCallbacks(worker::Dispatcher& View) : m_view(View)
	{
	}

	~ScopedViewCallbacks()
	{
		for (auto handle : m_handles)
		{
			m_view.Remove(handle);
		}
		m_handles.clear();
	}

	/** Add a handle that will be tracked and removed upon destruction. */
	void Add(uint64_t handle)
	{
		m_handles.emplace_back(handle);
	}

	/**
	* Remove a handle from tracking.
	* It is up to the caller to remove it from the View if it is still valid.
	*/
	void Remove(uint64_t handle)
	{
		m_handles.erase(std::remove(m_handles.begin(), m_handles.end(), handle), m_handles.end());
	}

private:
	worker::Dispatcher& m_view;
	std::vector<uint64_t> m_handles;
};

