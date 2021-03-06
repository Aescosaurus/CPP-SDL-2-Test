#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;

using ComponentID = std::size_t;

inline ComponentID getComponentTypeId()
{
	static ComponentID lastID = 0;
	return( lastID++ );
}

template<typename T>
inline ComponentID getComponentTypeId() noexcept
{
	static ComponentID typeId = getComponentTypeId();
	return( typeId );
}

constexpr std::size_t maxComponents = 32;

using ComponentBitSet = std::bitset<maxComponents>;
using ComponentArray = std::array<Component*,maxComponents>;

class Component
{
public:
	Entity* entity;

	virtual void init() {}
	virtual void update() {}
	virtual void draw() {}

	virtual ~Component() {}
};

class Entity
{
public:
	void update()
	{
		for( auto& c : components ) c->update();
	}
	void draw()
	{
		for( auto& c : components ) c->draw();
	}

	void destroy() { active = false; }
	template<typename T,typename... TArgs>
	T& addComponent( TArgs&&... mArgs )
	{
		T* c( new T( std::forward<TArgs>( mArgs )... ) );
		c->entity = this;
		std::unique_ptr<Component> uPtr{ c };
		components.emplace_back( std::move( uPtr ) );

		componentArray[getComponentTypeId<T>()] = c;
		componentBitSet[getComponentTypeId<T>()] = true;

		c->init();
		return( *c );
	}

	bool isActive() const { return( active ); }
	template<typename T>
	bool hasComponent()
	{
		return( componentBitSet[getComponentTypeId<T>()] );
	}
	template<typename T>
	T& getComponent() const
	{
		Component* ptr( componentArray[getComponentTypeId<T>()] );
		return( *static_cast<T*>( ptr ) );
	}
private:
	bool active = true;
	std::vector<std::unique_ptr<Component>> components;

	ComponentArray componentArray;
	ComponentBitSet componentBitSet;
};

class Manager
{
public:
	void update()
	{
		for( auto& entity : entities ) entity->update();
	}
	void draw()
	{
		for( auto& entity : entities ) entity->draw();
	}

	void refresh()
	{
		entities.erase( std::remove_if(
			std::begin( entities ),std::end( entities ),
			[]( const std::unique_ptr<Entity> &mEntity )
		{
			return( !mEntity->isActive() );
		} ),
			std::end( entities ) );
	}
	Entity& addEntity()
	{
		Entity* e = new Entity();
		std::unique_ptr<Entity> uPtr{ e };
		entities.emplace_back( std::move( uPtr ) );

		return( *e );
	}
private:
	std::vector<std::unique_ptr<Entity>> entities;
};