#pragma once
#include "../base/BaseAttributes.hpp"

#include <TermAPI.hpp>

#include <vector>
#include <string>

struct positionable_text : Positioned {
	std::vector<std::vector<std::string>>& lines;

	positionable_text(const point& csbTopMiddle, std::vector<std::vector<std::string>>& lines) : Positioned(csbTopMiddle), lines{ lines } {}

	size_t getLongestLine() const
	{
		size_t longest{ 0ull };
		for (const auto& it : lines)
			if (const auto& sz{ it.size() }; sz > longest)
				longest = sz;
		return longest;
	}

	size_t height() const
	{
		return lines.size();
	}

	friend std::ostream& operator<<(std::ostream& os, const positionable_text& txt)
	{
		os << term::SaveCursor;

		const auto& longest{ txt.getLongestLine() };
		point pos{ txt.getPos() };

		const auto& getpos{ [&pos](const size_t& line_len) -> point {
			point copy{ pos };
			++pos.y;
			const auto& halfLen{ line_len / 2 };
			copy.x -= halfLen - (halfLen % 2);
			return copy;
		} };

		for (const auto& l : txt.lines) {
			size_t len{ 0ull };
			for (const auto& w : l) {
				if (w.empty())
					continue;
				switch (w.at(0ull)) {
				case '\033':
					break;
				default:
					len += w.size();
					break;
				}
			}
			os << term::setCursorPosition(getpos(len));
			for (const auto& w : l)
				os << w;
		}

		return os << term::LoadCursor;
	}
};

