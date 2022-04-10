#pragma once
#include "../base/BaseAttributes.hpp"
#include "point.h"
#include "tile.h"

#include <xRand.hpp>

#include <array>
#include <memory>

struct GeneratorSettings {
	bool wall_always_on_edge{ true };
	float trap_chance{ 1.5f }, wall_chance{ 15.0f }, container_chance{ 0.2f };
	float trap_damage{ 10.0f };
	bool trap_piercing{ true };
};

class matrix {
public:
	position SizeX, SizeY, Size;
	GeneratorSettings generatorSettings;

protected:
	std::vector<std::unique_ptr<tile>> arr;

	void validateSize() const
	{
		if (arr.size() != static_cast<size_t>(SizeX * SizeY))
			throw make_exception("Matrix of size (", arr.size(), ") is invalid, expected (", Size, ')');
	}

	void generate(rng::Random& rng)
	{
		for (position y{ 0ll }; y < SizeY; ++y) {
			for (position x{ 0ll }; x < SizeX; ++x) {
				if (generatorSettings.wall_always_on_edge && y == 0 || y == SizeY - 1 || x == 0 || x == SizeX - 1) {
					arr.emplace_back(std::make_unique<walltile>());
				}
				else {
					const float v{ rng.get(0.0f, 100.0f) };
					if (v <= generatorSettings.trap_chance)
						arr.emplace_back(std::make_unique<traptile>(traptile{ generatorSettings.trap_damage, generatorSettings.trap_piercing }));
					else if (v <= generatorSettings.wall_chance)
						arr.emplace_back(std::make_unique<walltile>());
					else
						arr.emplace_back(std::make_unique<floortile>());
				}
			}
		}
	}

public:
	matrix(rng::Random& rng, const position& sizeX, const position& sizeY, const GeneratorSettings& generator_config = {}) : SizeX{ sizeX }, SizeY{ sizeY }, Size{ SizeX * SizeY }, generatorSettings{ generator_config } { generate(rng); }
	matrix(rng::Random& rng, const point& size, const GeneratorSettings& generator_config = {}) : SizeX{ size.x }, SizeY{ size.y }, Size{ SizeX * SizeY }, generatorSettings{ generator_config } { generate(rng); }

	/// @brief	Returns an iterator to the beginning of the container.
	[[nodiscard]] WINCONSTEXPR auto begin() const noexcept { return arr.begin(); }
	/// @brief	Returns an iterator to the end of the container.
	[[nodiscard]] WINCONSTEXPR auto end() const noexcept { return arr.end(); }
	/// @brief	Check if the container is empty.
	[[nodiscard]] WINCONSTEXPR auto empty() const noexcept { return arr.empty(); }
	/// @brief	Get the actual size of the container, not the expected virtual size.
	[[nodiscard]] WINCONSTEXPR auto actual_size() const noexcept { return arr.size(); }
	/// @brief	Get the current capacity of the container.
	[[nodiscard]] WINCONSTEXPR auto capacity() const noexcept { return arr.capacity(); }
	/// @brief	Change the reserved size of the container. If the specified capacity is lower than the current number of used elements, nothing happens.
	WINCONSTEXPR void reserve(const size_t& newCapacity) { arr.reserve(newCapacity); }
	/// @brief	Get the tile at the specified 1-dimensional index.
	[[nodiscard]] tile* at(const size_t& index) const noexcept(false) { return arr.at(index).get(); }

	[[nodiscard]] size_t to1D(const size_t& x, const size_t& y) const
	{
		return ::to1D(x, y, SizeX);
	}
	[[nodiscard]] size_t to1D(const point& pos) const
	{
		return ::to1D(pos.x, pos.y, SizeX);
	}
	[[nodiscard]] point from1D(const size_t& index) const
	{
		return ::from1D(index, SizeX);
	}

	[[nodiscard]] tile* get(const point& target) const
	{
		return arr.at(to1D(target)).get();
	}
	[[nodiscard]] tile* get(const size_t& x, const size_t& y) const
	{
		return arr.at(to1D(x, y)).get();
	}
	void set(const point& target, std::unique_ptr<tile>&& t)
	{
		arr.at(to1D(target)) = std::move(t);
	}
	void set(const size_t& x, const size_t& y, std::unique_ptr<tile>&& t)
	{
		arr.at(to1D(x, y)) = std::move(t);
	}
};
