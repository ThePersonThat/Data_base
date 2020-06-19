#pragma once
#include <stdlib.h>
#include "btree.h"
#include "constant.h"

/* Node Header Layout */

const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = sizeof(uint8_t);
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = sizeof(uint8_t) + sizeof(uint8_t);
const uint8_t COMMON_NODE_HEADER_SIZE = sizeof(uint8_t) * 4;

/* Leaf Node Header Layout */

const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = sizeof(uint32_t);
const uint32_t LEAF_NODE_HEADER_SIZE = sizeof(uint8_t) * 4 + sizeof(uint32_t);


/* Leaf Node Body Layout */

const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email);
const uint32_t LEAF_NODE_VALUE_OFFSET = 0 + sizeof(uint32_t);
const uint32_t LEAF_NODE_CELL_SIZE = sizeof(uint32_t) + size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email);
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - sizeof(uint8_t) * 4 + sizeof(uint32_t);
const uint32_t LEAF_NODE_MAX_CELLS = (PAGE_SIZE - sizeof(uint8_t) * 4 + sizeof(uint32_t)) / (sizeof(uint32_t) + size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email));

const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = ((PAGE_SIZE - sizeof(uint8_t) * 4 + sizeof(uint32_t)) / (sizeof(uint32_t) + size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email)) + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = ((PAGE_SIZE - sizeof(uint8_t) * 4 + sizeof(uint32_t)) / (sizeof(uint32_t) + size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email)) + 1) - ((PAGE_SIZE - sizeof(uint8_t) * 4 + sizeof(uint32_t)) / (sizeof(uint32_t) + size_of_attribute(Row, id) + size_of_attribute(Row, username) + size_of_attribute(Row, email)) + 1) / 2;

/*
	Internal Node Header Layout
*/

const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = sizeof(uint8_t) * 4;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_OFFSET = sizeof(uint32_t) + sizeof(uint8_t) * 4;
const uint32_t INTERNAL_NODE_HEADER_SIZE = sizeof(uint8_t) * 4 + sizeof(uint32_t) + sizeof(uint32_t);

/*
	Internal Node Body Layout
*/

const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE = sizeof(uint32_t) * 2;

uint32_t* leaf_node_num_cells(void* node)
{
	return (uint32_t*)((uintptr_t)node + LEAF_NODE_NUM_CELLS_OFFSET);
}

void* leaf_node_cell(void* node, uint32_t cell_num)
{
	return (void*)((uintptr_t)node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE);
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num)
{
	return leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num)
{
	return (void *)((uintptr_t)leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE);
}

void initialize_leaf_node(void* node)
{
	set_node_type(node, NODE_LEAF);
	set_node_root(node, 0);
	*leaf_node_num_cells(node) = 0;
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value)
{
	void* node = get_page(cursor->table->pager, cursor->page_num);

	uint32_t num_cells = *leaf_node_num_cells(node);

	if (num_cells >= LEAF_NODE_MAX_CELLS)
	{
		leaf_node_split_and_insert(cursor, key, value);
		return;
	}

	if (cursor->cell_num < num_cells)
	{
		for (uint32_t i = num_cells; i > cursor->cell_num; i--)
		{
			memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
		}
	}

	*(leaf_node_num_cells(node)) += 1;
	*leaf_node_key(node, cursor->cell_num) = key;
	serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key)
{
	void* node = get_page(table->pager, page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	Cursor* cursor = malloc(sizeof(Cursor));
	cursor->table = table;
	cursor->page_num = page_num;

	uint32_t min_index = 0;
	uint32_t one_past_max_index = num_cells;
	while (one_past_max_index != min_index)
	{
		uint32_t index = (min_index + one_past_max_index) / 2;
		uint32_t key_at_index = *leaf_node_key(node, index);
		if (key == key_at_index)
		{
			cursor->cell_num = index;
			return cursor;
		}

		if (key < key_at_index)
		{
			one_past_max_index = index;
		}
		else
		{
			min_index = index + 1;
		}
	}

	cursor->cell_num = min_index;
	return cursor;
}


void print_constants()
{
	printf("ROW_SIZE: %d\n", ROW_SIZE);
	printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
	printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
	printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
	printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
	printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

NodeType get_node_type(void* node)
{
	uint8_t value = *((uint8_t*)((uintptr_t)node + NODE_TYPE_OFFSET));
	return (NodeType)value;
}

void set_node_type(void* node, NodeType type)
{
	uint8_t value = type;
	*((uint8_t*)((uintptr_t)node + NODE_TYPE_OFFSET)) = value;
}

void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value)
{
	void* old_node = get_page(cursor->table->pager, cursor->page_num);
	uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
	void* new_node = get_page(cursor->table->pager, new_page_num);
	initialize_leaf_node(new_node);

	for (uint32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--)
	{
		void* destionation_node;
		if (i >= LEAF_NODE_LEFT_SPLIT_COUNT)
			destionation_node = new_node;
		else
			destionation_node = old_node;

		uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
		void* destionaton = leaf_node_cell(destionation_node, index_within_node);

		if (i == cursor->cell_num)
		{
			serialize_row(value, destionaton);
		}
		else if(i > cursor->cell_num)
		{
			memcpy(destionaton, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
		}
		else
		{
			memcpy(destionaton, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
		}
	}

	*(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
	*(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

	if (is_node_root(old_node)) 
	{
		return create_new_root(cursor->table, new_page_num);
	}
	else
	{
		printf("Need to implement updating parent after split.\n");
		exit(EXIT_FAILURE);
	}
}

void create_new_root(Table* table, uint32_t right_child_page_num)
{
	void* root = get_page(table->pager, table->root_page_num);
	void* right_child = get_page(table->pager, right_child_page_num);
	uint32_t left_child_page_num = get_unused_page_num(table->pager);
	void* left_child = get_page(table->pager, left_child_page_num);
	memcpy(left_child, root, PAGE_SIZE);
	set_node_root(left_child, 0);
	initialize_internal_node(root);
	set_node_root(root, 1);
	*internal_node_num_keys(root) = 1;
	*internal_node_child(root, 0) = left_child_page_num;
	uint32_t left_child_max_key = get_node_max_key(left_child);
	*internal_node_key(root, 0) = left_child_max_key;
	*internal_node_right_child(root) = right_child_page_num;
}

uint32_t* internal_node_num_keys(void* node)
{
	return (uint32_t*)((uintptr_t)node + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* internal_node_right_child(void* node)
{
	return (uint32_t*)((uintptr_t)node + INTERNAL_NODE_RIGHT_OFFSET);
}

uint32_t* internal_node_cell(void* node, uint32_t cell_num)
{
	return (uint32_t*)((uintptr_t)node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* internal_node_child(void* node, uint32_t child_num)
{
	uint32_t num_keys = *internal_node_num_keys(node);
	if (child_num > num_keys)
	{
		printf("Tried to access child_num %d > num_keys %d\n", child_num, num_keys);
		exit(EXIT_FAILURE);
	}
	else if (child_num == num_keys)
		return internal_node_right_child(node);
	else
		return internal_node_cell(node, child_num);
}

uint32_t* internal_node_key(void* node, uint32_t key_num)
{
	return internal_node_cell(node, key_num) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t get_node_max_key(void* node)
{
	switch (get_node_type(node))
	{
	case NODE_INTERNAL:
		return *internal_node_key(node, *internal_node_num_keys(node) - 1);
	case NODE_LEAF:
		return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
	}
}

uint8_t is_node_root(void* node)
{
	uint8_t value = *((uint8_t*)((uintptr_t)node + IS_ROOT_OFFSET));
	return value;
}

void* set_node_root(void* node, uint8_t is_root)
{
	uint8_t value = is_root;
	*((uint8_t*)((uintptr_t)node + IS_ROOT_OFFSET)) = value;
}

void initialize_internal_node(void* node)
{
	set_node_type(node, NODE_INTERNAL);
	set_node_root(node, 0);
	*internal_node_num_keys(node) = 0;
}

void indent(uint32_t level)
{
	for (uint32_t i = 0; i < level; i++)
		printf("  ");
}

void print_tree(Pager* pager, uint32_t page_num, uint32_t indetantion_level)
{
	void* node = get_page(pager, page_num);
	uint32_t num_keys, child;

	switch (get_node_type(node))
	{
	case NODE_LEAF:
		num_keys = *leaf_node_num_cells(node);
		indent(indetantion_level);
		printf("- leaf (size %d)\n", num_keys);

		for (uint32_t i = 0; i < num_keys; i++)
		{
			indent(indetantion_level + 1);
			printf("- %d\n", *leaf_node_key(node, i));
		}
		break;
	case NODE_INTERNAL:
		num_keys = *internal_node_num_keys(node);
		indent(indetantion_level);
		printf("- internal (size %d)\n", num_keys);
		for (uint32_t i = 0; i < num_keys; i++)
		{
			child = *internal_node_child(node, i);
			print_tree(pager, child, indetantion_level + 1);
			indent(indetantion_level + 1);
			printf("- key %d\n", *internal_node_key(node, i));
		}
		child = *internal_node_right_child(node);
		print_tree(pager, child, indetantion_level + 1);
		break;
	}
}