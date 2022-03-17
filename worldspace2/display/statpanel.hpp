#pragma once
#include "../base/BaseAttributes.hpp"
#include "../actors/ActorBase.hpp"
#include "frame.hpp"
#include "../display/positionable_text.h"

#include <concepts>

template<typename T>
struct statbar {
	StatBase<T>* stat{ nullptr };
	int scale{ 10 };
	char left{ '(' }, right{ ')' }, fill{ '@' }, empty{ ' ' };
	color::setcolor fillColor{ color::white, color::Layer::B };

	statbar(StatBase<T>* stat, const color::setcolor& fillColor) : stat{ stat }, fillColor{ fillColor } {}

	friend std::ostream& operator<<(std::ostream& os, const statbar<T>& sb)
	{
		const int& scale{ sb.scale };
		int fill{ 0 };
		if (sb.stat != nullptr)
			fill = sb.stat->toScale(scale);
		os << sb.left << sb.fillColor;
		for (int i{ 0 }; i < scale; ++i) {
			if (fill >= i)
				os << sb.fill;
			else {
				os << std::string(scale - i, sb.empty);
				break;
			}
		}
		os << color::reset << sb.right;
		return os;
	}

	operator StatBase<T>* () const { return stat; }
};

inline static constexpr const int STATPANEL_HEIGHT{ 2 }, STATPANEL_PADDING{ 2 };

struct statpanel {
	position originRow;
	ActorBase* actor;
	statbar<float> hp, sp;
	std::string separator{ "   " };

	statpanel(const position& originRow, ActorBase* actor) : originRow{ originRow }, actor{ actor }, hp{ &actor->health, color::setcolor::red }, sp{ &actor->stamina, color::setcolor::green } {}

	void display() noexcept(false)
	{
		if (actor == nullptr || hp == nullptr || sp == nullptr)
			throw make_exception("statpanel::display() failed:  Stat pointers are null!");

		const point csbSize{ term::getScreenBufferSize() };
		const position centerColumn{ (csbSize.x / 2) - 2 };

		position ln{ originRow };

		const auto& fst_line{ str::stringify(actor->name, "   level ", actor->level) };

		std::cout << term::setCursorPosition(centerColumn - static_cast<position>(fst_line.size() / 2ull), ln++) << fst_line;

		const point& statbar_origin{ centerColumn - static_cast<position>(hp.scale + sp.scale + separator.size()) / 2ll, ln++ };
		std::cout << term::setCursorPosition(statbar_origin) << hp << separator << sp;
	}
};

template<typename StatType>
struct positionable_statbar : positionable_text {
	std::vector<std::vector<std::string>> text;
	char left{ '(' }, right{ ')' }, fill{ '@' }, mod{ '@' }, empty{ ' ' };
	color::setcolor fillColor, modColor;
	int scale{ 10 };

	StatBase<StatType>* stat{ nullptr };

	positionable_statbar(StatBase<StatType>* stat, const point& csbTopMiddle, const color::setcolor& color) : positionable_text(csbTopMiddle, text), fillColor{ color }, modColor{}, stat{ stat } {}

	std::string getFill() const
	{
		std::string buf;
		if (stat != nullptr) {
			buf.reserve(static_cast<size_t>(scale * 2));
			const auto& scaled{ stat->toScale() };
			for (int i{ 0 }; i < scale; ++i) {

			}
		}
		buf.shrink_to_fit();
		return buf;
	}
};

