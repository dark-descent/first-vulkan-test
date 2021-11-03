#ifndef ENGINE_TERMINATABLE_HPP
#define ENGINE_TERMINATABLE_HPP

class Terminatable
{
private:
	bool isTerminated_ = false;

protected:
	virtual bool onTerminate() = 0;

public:
	bool terminate()
	{
		if (!isTerminated_)
		{
			if(!onTerminate())
				return false;

			isTerminated_ = true;
		}
		return true;
	}

	bool isTerminated() { return this->isTerminated_; }
};

#endif
