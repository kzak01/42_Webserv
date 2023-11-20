#ifndef TEMPLATES_TPP
# define TEMPLATES_TPP
# include "includes.hpp"
# include "types.hpp"

template<typename Iterator>
void it_advance(Iterator& it, size_t amount)
{
	for (size_t x = 0; x < amount; x++)
		++it;
}

template<typename Iterator>
Iterator& it_previous(Iterator& it, Iterator& prev)
{
	prev = it;
	--prev;
	if (prev != Iterator())
		return(it);
	else
		return (prev);
}

template<typename Container>
void printContainer(const Container& container) {
	std::cout << "-------CONTAINER-------" << std::endl;
	typename Container::const_iterator it;
	for (it = container.begin(); it != container.end(); ++it) {
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	std::cout << "-----------------------" << std::endl;
}

#endif
