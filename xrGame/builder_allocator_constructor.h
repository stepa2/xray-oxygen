////////////////////////////////////////////////////////////////////////////
//	Module 		: builder_allocator_constructor.h
//	Created 	: 21.03.2002
//  Modified 	: 28.02.2004
//	Author		: Dmitriy Iassenev
//	Description : Builder allocator constructor
////////////////////////////////////////////////////////////////////////////

#pragma once

template <typename _path_builder, typename _vertex_allocator>
struct CBuilderAllocatorConstructor 
{
	template <template <typename _T> class _vertex> 
	class CDataStorage : 
		public _path_builder::template CDataStorage<_vertex>,
		public _vertex_allocator::template CDataStorage<typename _path_builder::template CDataStorage<_vertex>::CGraphVertex>
	{
	public:
		typedef typename _path_builder::template CDataStorage<_vertex>	CDataStorageBase;
		typedef typename _vertex_allocator::template CDataStorage<
			typename _path_builder::template CDataStorage<
				_vertex
			>::CGraphVertex
		>												CDataStorageAllocator;
		typedef typename CDataStorageBase::CGraphVertex CGraphVertex;
		typedef typename CGraphVertex::_index_type		_index_type;

	public:
		IC		CDataStorage(const u32 vertex_count) : CDataStorageBase(vertex_count), CDataStorageAllocator() {};
		virtual	~CDataStorage	() = default;
		IC void	init			() { CDataStorageBase::init(); CDataStorageAllocator::init(); }
	};
};