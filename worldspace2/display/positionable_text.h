#pragma once
#include "../base/BaseAttributes.hpp"

#include <TermAPI.hpp>

#include <vector>
#include <string>

struct positionable_text : Positioned {
	std::vector<std::string>& lines;

	positionable_text(const point& csbTopMiddle, std::vector<std::string>& lines) : Positioned(csbTopMiddle), lines{ lines } {}

	size_t getLongestLine() const
	{
		size_t longest{ 0ull };
		for (const auto& it : lines)
			if (const auto& sz{ it.size() }; sz > longest)
				longest = sz;
		return longest;
	}

	friend std::ostream& operator<<(std::ostream& os, const positionable_text& txt)
	{
		os << term::SaveCursor;
		
		const auto& longest{ txt.getLongestLine() };
		point pos{ txt.pos };

		const auto& getpos{ [&pos](const size_t& line_len) -> point {
			point copy{ pos };
			++pos.y;
			const auto& halfLen{ line_len / 2 };
			copy.x -= halfLen - (halfLen % 2);
			return copy;
		} };

		for (const auto& l : txt.lines)
			os << term::setCursorPosition(getpos(l.size())) << l;

		return os << term::LoadCursor;
	}
};

