/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file LazyPriorityQueue.hpp
 * 
 * @brief Generic lazy priority queue implementation that allows
 * updating priorities by pushing duplicates and lazily deleting
 * invalid elements during pop.
 * 
 */

#pragma once
#include <queue>
#include <vector>
#include <functional>

/*
* @tparam T The data type to store.
* @tparam Compare The comparrator type to determine priority.
*/
template<typename T, typename Compare = std::less<T>>
class LazyPriorityQueue {
private:
	std::priority_queue<T, std::vector<T>, Compare> prioQueue;

public:
	void push(const T& element)
	{
		prioQueue.push(element);
	}

	bool empty() const
	{
		return prioQueue.empty();
	}

	/*
	* @brief Pops and deletes the highest prio valid element.
	* invalid elements are silently removed.
	* 
	* @param outElement The output parameter to store the popped element.
	* @param isValid A function to determine if an element is valid or should be discarded.
	* @return true if valid element was found, false if queue is out of valid elements
	*/
	bool popValid(T& outElement, std::function<bool(const T&)> isValid)
	{
		while (!prioQueue.empty())
		{
			// get highest prio element
			T top = prioQueue.top();
			// pop it from the queue
			prioQueue.pop();

			// if valid then return the element, otherwise discard and continue
			if (isValid(top))
			{
				outElement = top;
				return true;
			}
		}

		return false;
	}
};

 /* End of the LazyPriorityQueue.hpp file */