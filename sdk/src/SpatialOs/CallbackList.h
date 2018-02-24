#pragma once

#include <map>

#define DECLARE_CALLBACK_LIST(NAME, FNNAME, ARGS) \
	private:\
		CCallbackList<size_t, ARGS> NAME; \
	public:\
		size_t Add##FNNAME##Callback(std::function<void(ARGS)> callback) { return NAME.Add(callback); } \
		bool Remove##FNNAME##Callback(size_t key) { return NAME.Remove(key); } \
	private:

template <typename C, typename... Args>
class CCallbackList
{
public:
	CCallbackList() : m_lastKey(1) {}
	C Add(std::function<void(Args...)> callback)
	{
		C key;
		if (m_usedKeys.size() > 0)
		{
			key = m_usedKeys.front();
			m_usedKeys.pop();
		}
		else
		{
			key = m_lastKey++;
		}
		m_pendingInserts.push_back(std::make_pair(key, callback));
		return key;
	}

	bool Remove(C key)
	{
		auto it = m_callbacks.find(key);
		if (it != m_callbacks.end())
		{
			// Remove later in case this removal is in an Update
			m_pendingUsedKeys.push_back(key);
			return true;
		}
		return false;
	}

	void Update(Args... args)
	{
		Process();
		for (auto& pair : m_callbacks)
		{
			pair.second(args...);
		}
		Process();
	}

private:

	void Process()
	{
		// Process removals
		for (auto key : m_pendingUsedKeys)
		{
			m_callbacks.erase(key);
			m_usedKeys.push(key);
		}
		m_pendingUsedKeys.clear();
		// Process additions
		for (auto& kvp : m_pendingInserts)
		{
			m_callbacks.emplace(kvp.first, kvp.second);
		}
		m_pendingInserts.clear();
	}

	std::map<C, std::function<void(Args...)>> m_callbacks;
	std::queue<C> m_usedKeys;
	std::vector<C> m_pendingUsedKeys;
	std::vector<std::pair<C, std::function<void(Args...)>>> m_pendingInserts;
	C m_lastKey;
 };
