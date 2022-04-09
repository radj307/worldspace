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

template<typename StatType>
struct positionable_statbar : positionable_text {
	std::vector<std::vector<std::string>> text;
	char left{ '(' }, right{ ')' }, fill{ '@' }, mod{ '@' }, empty{ ' ' };
	color::setcolor fillColor, modColor;
	int scale{ 10 };

	StatBase<StatType>* stat{ nullptr };

	positionable_statbar(StatBase<StatType>* stat, const point& csbTopMiddle, const color::setcolor& color) : positionable_text(csbTopMiddle, text), fillColor{ color }, modColor{}, stat{ stat } {}

	std::string getBar() const
	{
		const auto& scaled{ stat->toScale(scale) };
		const auto& withMod{ stat->toScaleWithModifier(scale) };
		return str::stringify(left, fillColor, std::string(scaled, fill), color::reset, modColor, std::string(withMod - scaled, mod), color::reset, std::string(withMod - scaled, empty), color::reset, right);
	}

	operator StatBase<StatType>* () const { return stat; }

	friend std::ostream& operator<<(std::ostream& os, const positionable_statbar<StatType>& psb)
	{
		return os << psb.getBar();
	}
};

struct statpanel {
	position originRow;
	ActorBase* actor;
	statbar<float> hp, sp;

	position centerCol{ -1 }, nameCol{ -1 }, levelCol{ -1 };

	statpanel(const position& originRow, ActorBase* actor) :
		originRow{ originRow },
		actor{ actor },
		hp{ &actor->health, color::setcolor::red },
		sp{ &actor->stamina, color::setcolor::green }
	{}

	void initPositions()
	{
		centerCol = static_cast<point>(term::getScreenBufferSize()).x / 2 - 2;
		nameCol = (centerCol - 1) - hp.scale / 2 - 1;
		levelCol = (centerCol + 2) + sp.scale / 2 + 1;
	}

	void display() noexcept(false)
	{
		if (actor == nullptr || hp == nullptr || sp == nullptr)
			throw make_exception("statpanel::display() failed:  Stat pointers are null!");

		position ln{ originRow };
		
		std::cout << term::setCursorPosition(nameCol - actor->name.size() / 2, ln) << actor->name;
		const auto& lvlstr{ "Level " + std::to_string(actor->level)};
		std::cout << term::setCursorPosition(levelCol - lvlstr.size() / 2, ln) << lvlstr;

		//const point csbSize{ term::getScreenBufferSize() };
		//const position centerColumn{ (csbSize.x / 2) - 2 };


		//const auto& nameColumn{ (centerColumn - 1) - hp.scale / 2 };

		//std::cout << term::setCursorPosition(nameColumn - actor->name.size() - 2, ln) << actor->name;
		//std::cout << term::setCursorPosition(centerColumn + 5, ln) << "Level " << actor->level;

		++ln;

		const point& statbar_origin{ centerCol - static_cast<position>(hp.scale + sp.scale + 3) / static_cast<position>(2), ln++ };
		std::cout << term::setCursorPosition(statbar_origin) << hp << "   " << sp;
	}
};

