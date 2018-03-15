#ifndef _Ascend_H_
#define _Ascend_H_
template<class T>
class Ascend //Ascending order
{
public:
	bool operator ()(const T& item1, const T& item2);
};

template<class T>
bool Ascend<T>::operator ()(const T& item1, const T& item2)
{
	return (item1 < item2); //The more weight, the higher rank
}
#endif