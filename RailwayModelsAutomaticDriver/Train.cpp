#include "Train.hpp"
#include "Section.hpp"

void Train::putTrain(Section& section) {
	section.train = this;
	section.is_occupied = SECTION_ENTERED;
}
