#pragma once
#include "point.h"
#include "tile.h"

#include <array>
#include <memory>

template<size_t SizeX, size_t SizeY>
class matrix {
public:
	static constexpr const size_t SIZE{ SizeX * SizeY };
	using elem_type = std::unique_ptr<tile>;
	using container_type = std::array<elem_type, SIZE>;

private:
	container_type arr;

	void validateSize() const
	{
		if (arr.size() != SizeX * SizeY)
			throw make_exception();
	}

public:
	matrix() = default;
	matrix(container_type&& arr) : arr{ std::move(arr) } {}
	matrix(const container_type& arr) : arr{ arr } {}

	size_t to1D(const size_t& x, const size_t& y) const
	{
		return x + y * SizeX;
	}
	size_t to1D(const point& pos) const
	{
		return to1D(pos.x, pos.y);
	}
	point from1D(const size_t& index) const
	{
		point pos;
		pos.x = index / SizeX;
		pos.y = index % SizeX;
		return pos;
	}

	tile* get(const point& target) const
	{
		return arr.at(to1D(target)).get();
	}
	tile* get(const size_t& x, const size_t& y) const
	{
		return arr.at(to1D(x, y)).get();
	}
	void set(const point& target, tile&& t)
	{
		arr.at(to1D(target)) = std::make_unique<tile>(std::move(t));
	}
	void set(const size_t& x, const size_t& y, tile&& t)
	{
		arr.at(to1D(x, y)) = std::make_unique<tile>(std::move(t));
	}
	void set(const point& target, const tile& t)
	{
		arr.at(to1D(target)) = std::make_unique<tile>(t);
	}
	void set(const size_t& x, const size_t& y, const tile& t)
	{
		arr.at(to1D(x, y)) = std::make_unique<tile>(t);
	}
};
