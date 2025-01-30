/*
 * 红黑树启动内核实现。
 * @date 2025-01-03
 *
 * Copyright (c) 2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
#include "include/boot_tree.h"
#include <stdbool.h>

/*
 * 红黑树找祖宗结点。
 *
 * @param pool 位映射池。
 * @param node 查询结点。
 *
 * @return 返回找到的祖宗结点句柄，或未定义。
 */
static handle boot_tree_get_grandparent(bitmap_pool* restrict pool,const tree_node* restrict node)
{
	if(node->parent!=HANDLE_UNDEFINED)
	{
		tree_node* parent=boot_bitmap_pool_content(pool,node->parent);
		if((uintn)parent==HANDLE_UNDEFINED)
		{
			return HANDLE_UNDEFINED;
		}
		else
		{
			return parent->parent;
		}
	}
	else
	{
		return HANDLE_UNDEFINED;
	}
}

/*
 * 红黑树结点左旋。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param node 结点句柄。
 *
 * @return 无返回值。
 */
static void boot_tree_rotate_left(bitmap_pool* restrict pool,handle* restrict root,const handle node)
{
	tree_node* nodec=boot_bitmap_pool_content(pool,node);
	handle right=nodec->right;
	tree_node* rightc=boot_bitmap_pool_content(pool,right);

	nodec->right=rightc->left;
	if(rightc->left!=HANDLE_UNDEFINED)
	{
		tree_node* rleftc=boot_bitmap_pool_content(pool,rightc->left);
		rleftc->parent=node;
	}

	rightc->parent=nodec->parent;

	if(nodec->parent!=HANDLE_UNDEFINED)
	{
		tree_node* nparentc=boot_bitmap_pool_content(pool,nodec->parent);
		if(node==nparentc->left)
		{
			nparentc->left=right;
		}
		else
		{
			nparentc->right=right;
		}
	}
	else
	{
		*root=right;
	}

	rightc->left=node;
	nodec->parent=right;
}

/*
 * 红黑树结点右旋。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param node 结点句柄。
 *
 * @return 无返回值。
 */
static void boot_tree_rotate_right(bitmap_pool* restrict pool,handle* restrict root,const handle node)
{
	tree_node* nodec=boot_bitmap_pool_content(pool,node);
	handle left=nodec->left;
	tree_node* leftc=boot_bitmap_pool_content(pool,left);

	nodec->left=leftc->right;
	if(leftc->right!=HANDLE_UNDEFINED)
	{
		tree_node* lrightc=boot_bitmap_pool_content(pool,leftc->right);
		lrightc->parent=node;
	}

	leftc->parent=nodec->parent;

	if(nodec->parent!=HANDLE_UNDEFINED)
	{
		tree_node* nparentc=boot_bitmap_pool_content(pool,nodec->parent);
		if(node==nparentc->right)
		{
			nparentc->right=left;
		}
		else
		{
			nparentc->left=left;
		}
	}
	else
	{
		*root=left;
	}

	leftc->right=node;
	nodec->parent=left;
}

/*
 * 红黑树结点插入重平衡。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param node 结点句柄。
 *
 * @return 无返回值。
 */
static void boot_tree_insert_rebalance(bitmap_pool* restrict pool,handle* restrict root,handle node)
{
	tree_node* nodec=boot_bitmap_pool_content(pool,node);
	handle nparent=nodec->parent;
	tree_node* nparentc=boot_bitmap_pool_content(pool,nparent);

	if(nparent!=HANDLE_UNDEFINED&&nparentc->color)
	{
		handle current=node;
		tree_node* currentc=nodec;
		tree_node* cparentc=nparentc;

		while(current!=*root&&current!=HANDLE_UNDEFINED&&currentc->parent!=HANDLE_UNDEFINED&&cparentc->color)
		{
			handle parent=currentc->parent;
			tree_node* parentc=cparentc;
			handle grandparent=boot_tree_get_grandparent(pool,currentc);
			tree_node* grandparentc=boot_bitmap_pool_content(pool,grandparent);
			handle uncle=HANDLE_UNDEFINED;
			tree_node* unclec=(void*)HANDLE_UNDEFINED;
			bool is_left;

			if(parent==grandparentc->left)
			{
				is_left=false;
				uncle=grandparentc->right;
			}
			else
			{
				is_left=true;
				uncle=grandparentc->left;
			}
			unclec=boot_bitmap_pool_content(pool,uncle);

			if(uncle!=HANDLE_UNDEFINED&&unclec->color)
			{
				/*情况1：红叔叔*/
				parentc->color=TREE_BLACK;
				unclec->color=TREE_BLACK;
				grandparentc->color=TREE_RED;
				current=grandparent;
				currentc=grandparentc;
				cparentc=boot_bitmap_pool_content(pool,grandparentc->parent);
			}
			else
			{
				/*情况2：黑叔叔红家长*/
				if(!is_left&&parentc->right==current)
				{
					current=currentc->parent;
					boot_tree_rotate_left(pool,root,current);
					currentc=boot_bitmap_pool_content(pool,current);
				}
				else if(is_left&&parentc->left==current)
				{
					current=currentc->parent;
					boot_tree_rotate_right(pool,root,current);
					currentc=boot_bitmap_pool_content(pool,current);
				}

				parent=currentc->parent;
				parentc=boot_bitmap_pool_content(pool,parent);
				cparentc=parentc;
				parentc->color=TREE_BLACK;
				grandparent=boot_tree_get_grandparent(pool,currentc);
				grandparentc=boot_bitmap_pool_content(pool,grandparent);
				grandparentc->color=TREE_RED;
				if(is_left)
				{
					boot_tree_rotate_left(pool,root,grandparent);
				}
				else
				{
					boot_tree_rotate_right(pool,root,grandparent);
				}
			}
		}

		tree_node* rootc=boot_bitmap_pool_content(pool,*root);
		rootc->color=TREE_BLACK;
	}
}

/*
 * 红黑树结点插入。
 *
 * @param pool	  位映射池。
 * @param root	  树根结点。
 * @param compare 比较函数。
 * @param node	  结点句柄。
 *
 * @return 成功返回真，失败返回假。
 */
extern bool boot_tree_insert(bitmap_pool* restrict pool,handle* restrict root,const boot_tree_compare compare,const handle node)
{
	if(node==HANDLE_UNDEFINED)
	{
		return false;
	}

	if(*root==HANDLE_UNDEFINED)
	{
		*root=node;
		tree_node* rootc=boot_bitmap_pool_content(pool,*root);
		rootc->color=TREE_BLACK;
		return true;
	}

	handle current=*root;
	tree_node* currentc=boot_bitmap_pool_content(pool,*root);
	tree_node* nodec=boot_bitmap_pool_content(pool,node);

	while(current!=HANDLE_UNDEFINED)
	{
		int comp=compare((handle)currentc,(handle)nodec);

		if(comp==0)
		{
			/*不允许第二个相同存在*/
			return false;
		}
		if(comp>0)
		{
			if(currentc->left==HANDLE_UNDEFINED)
			{
				currentc->left=node;
				break;
			}
			else
			{
				current=currentc->left;
				currentc=boot_bitmap_pool_content(pool,current);
			}
		}
		else
		{
			if(currentc->right==HANDLE_UNDEFINED)
			{
				currentc->right=node;
				break;
			}
			else
			{
				current=currentc->right;
				currentc=boot_bitmap_pool_content(pool,current);
			}
		}
	}

	nodec->parent=current;
	boot_tree_insert_rebalance(pool,root,node);
	return true;
}

/*
 * 红黑树寻找最小结点。
 *
 * @param pool 位映射池。
 * @param node 结点句柄。
 *
 * @return 返回最小结点。
 */
static handle boot_tree_find_minimum(bitmap_pool* restrict pool,const handle node)
{
	handle current=node;
	tree_node* currentc=boot_bitmap_pool_content(pool,current);

	while(currentc->left!=HANDLE_UNDEFINED)
	{
		current=currentc->left;
		currentc=boot_bitmap_pool_content(pool,current);
	}
	return current;
}

/*
 * 红黑树寻找最大结点。
 *
 * @param pool 位映射池。
 * @param node 结点句柄。
 *
 * @return 返回最大结点。
 */
static handle boot_tree_find_maximum(bitmap_pool* restrict pool,const handle node)
{
	handle current=node;
	tree_node* currentc=boot_bitmap_pool_content(pool,current);

	while(currentc->right!=HANDLE_UNDEFINED)
	{
		current=currentc->right;
		currentc=boot_bitmap_pool_content(pool,current);
	}
	return current;
}

/*
 * 红黑树寻找前驱结点。
 *
 * @param pool 位映射池。
 * @param node 结点句柄。
 *
 * @return 返回前驱结点。
 */
static handle boot_tree_find_predecessor(bitmap_pool* restrict pool,handle node)
{
	tree_node* nodec=boot_bitmap_pool_content(pool,node);

	if(nodec->right!=HANDLE_UNDEFINED)
	{
		return boot_tree_find_maximum(pool,nodec->left);
	}

	handle parent=nodec->parent;
	tree_node* parentc=boot_bitmap_pool_content(pool,parent);

	while(parent!=HANDLE_UNDEFINED&&node==parentc->left)
	{
		node=parent;
		parent=parentc->parent;
		parentc=boot_bitmap_pool_content(pool,parent);
	}
	return parent;
}

/*
 * 红黑树寻找后继结点。
 *
 * @param pool 位映射池。
 * @param node 结点句柄。
 *
 * @return 返回后继结点。
 */
static handle boot_tree_find_successor(bitmap_pool* restrict pool,handle node)
{
	tree_node* nodec=boot_bitmap_pool_content(pool,node);

	if(nodec->right!=HANDLE_UNDEFINED)
	{
		return boot_tree_find_minimum(pool,nodec->right);
	}

	handle parent=nodec->parent;
	tree_node* parentc=boot_bitmap_pool_content(pool,parent);

	while(parent!=HANDLE_UNDEFINED&&node==parentc->right)
	{
		node=parent;
		parent=parentc->parent;
		parentc=boot_bitmap_pool_content(pool,parent);
	}
	return parent;
}

/*
 * 红黑树结点交换。将b换至a的位置
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param a	   结点a句柄。
 * @param b	   结点b句柄。
 *
 * @return 无返回值。
 */
static void boot_tree_swap(bitmap_pool *restrict pool,handle* restrict root,const handle a,const handle b)
{
	tree_node* ac=boot_bitmap_pool_content(pool,a);
	tree_node* bc=boot_bitmap_pool_content(pool,b);
	handle left=ac->left;
	handle right=ac->right;
	handle parent=ac->parent;

	bc->parent=parent;

	if(parent!=HANDLE_UNDEFINED)
	{
		tree_node* parentc=boot_bitmap_pool_content(pool,parent);
		if(a==parentc->left)
		{
			parentc->left=b;
		}
		else
		{
			parentc->right=b;
		}
	}
	else
	{
		if(*root==a)
		{
			*root=b;
		}
	}

	bc->right=right;
	if(right!=HANDLE_UNDEFINED)
	{
		tree_node* rightc=boot_bitmap_pool_content(pool,right);
		rightc->parent=b;
	}

	ac->right=HANDLE_UNDEFINED;
	bc->left=left;
	if(left!=HANDLE_UNDEFINED)
	{
		tree_node* leftc=boot_bitmap_pool_content(pool,left);
		leftc->parent=b;
	}

	ac->left=HANDLE_UNDEFINED;
	bc->color=ac->color;
	ac->parent=HANDLE_UNDEFINED;
}

/*
 * 红黑树结点删除重平衡。
 *
 * @param pool	 位映射池。
 * @param root	 树根结点。
 * @param parent 家长结点句柄。
 * @param node	 结点句柄。
 * @param isleft 结点是否在左边。
 *
 * @return 无返回值。
 */
static void boot_tree_remove_rebalance(bitmap_pool *restrict pool,handle* restrict root,handle parent,handle node,bool isleft)
{
	tree_node* nodec=boot_bitmap_pool_content(pool,node);

	/*非根黑结点*/
	while(*root!=node&&(node==HANDLE_UNDEFINED||!nodec->color))
	{
		tree_node* parentc=boot_bitmap_pool_content(pool,parent);

		handle sibling=isleft?parentc->right:parentc->left;
		tree_node* siblingc=boot_bitmap_pool_content(pool,sibling);

		if(sibling!=HANDLE_UNDEFINED&&siblingc->color)
		{
			/*情况1：红兄弟*/
			siblingc->color=TREE_BLACK;
			parentc->color=TREE_RED;

			if(isleft)
			{
				boot_tree_rotate_left(pool,root,parent);
			}
			else
			{
				boot_tree_rotate_right(pool,root,parent);
			}

			sibling=isleft?parentc->right:parentc->left;
			siblingc=boot_bitmap_pool_content(pool,sibling);
		}

		handle sleft=sibling==HANDLE_UNDEFINED?HANDLE_UNDEFINED:siblingc->left;
		handle sright=sibling==HANDLE_UNDEFINED?HANDLE_UNDEFINED:siblingc->right;
		tree_node* sleftc=boot_bitmap_pool_content(pool,sleft);
		tree_node* srightc=boot_bitmap_pool_content(pool,sright);

		if((sleft==HANDLE_UNDEFINED||!sleftc->color)&&(sright==HANDLE_UNDEFINED||!srightc->color))
		{
			/*情况2：兄弟双黑结点*/
			if(sibling!=HANDLE_UNDEFINED)
			{
				siblingc->color=TREE_RED;
			}

			node=parent;
			nodec=parentc;
			parent=parentc->parent;
			parentc=boot_bitmap_pool_content(pool,parent);
			isleft=parent!=HANDLE_UNDEFINED&&parentc->left==node;
		}
		else
		{
			if(isleft&&(sright==HANDLE_UNDEFINED||!srightc->color))
			{
				/*情况3：右兄弟左红右黑*/
				siblingc->color=TREE_RED;
				if(sleft!=HANDLE_UNDEFINED)
				{
					sleftc->color=TREE_BLACK;
				}

				boot_tree_rotate_right(pool,root,sibling);
				sibling=parentc->right;
				siblingc=boot_bitmap_pool_content(pool,sibling);
			}
			else if(!isleft&&(sleft==HANDLE_UNDEFINED||!sleftc->color))
			{
				/*情况4：左兄弟左黑右红*/
				siblingc->color=TREE_RED;
				if(sright!=HANDLE_UNDEFINED)
				{
					srightc->color=TREE_BLACK;
				}

				boot_tree_rotate_left(pool,root,sibling);
				sibling=parentc->left;
				siblingc=boot_bitmap_pool_content(pool,sibling);
			}

			/*情况5*/
			sleft=siblingc->left;
			sright=siblingc->right;
			siblingc->color=parentc->color;
			parentc->color=TREE_BLACK;

			if(isleft&&sright!=HANDLE_UNDEFINED)
			{
				srightc=boot_bitmap_pool_content(pool,sright);
				srightc->color=TREE_BLACK;
				boot_tree_rotate_left(pool,root,parent);
			}
			else if(!isleft&&sleft!=HANDLE_UNDEFINED)
			{
				sleftc=boot_bitmap_pool_content(pool,sleft);
				sleftc->color=TREE_BLACK;
				boot_tree_rotate_right(pool,root,parent);
			}

			node=*root;
			nodec=boot_bitmap_pool_content(pool,node);
		}
	}

	if(node!=HANDLE_UNDEFINED)
	{
		nodec->color=TREE_BLACK;
	}
}

/*
 * 红黑树结点删除。使用者有责任保证结点在树内。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 * @param node 结点句柄。
 *
 * @return 无返回值。
 */
extern void boot_tree_remove(bitmap_pool* restrict pool,handle* restrict root,handle node)
{
	handle next;
	tree_node* nextc;
	tree_node* nodec=boot_bitmap_pool_content(pool,node);

	if(nodec->left==HANDLE_UNDEFINED||nodec->right==HANDLE_UNDEFINED)
	{
		next=node;
		nextc=nodec;
	}
	else
	{
		next=boot_tree_find_successor(pool,node);
		nextc=boot_bitmap_pool_content(pool,next);
	}

	handle current;
	handle parent;

	if(nextc->left!=HANDLE_UNDEFINED)
	{
		current=nextc->left;
	}
	else
	{
		current=nextc->right;
	}
	tree_node* currentc=boot_bitmap_pool_content(pool,current);

	if(current!=HANDLE_UNDEFINED)
	{
		currentc->parent=nextc->parent;
		parent=currentc->parent;
	}
	else
	{
		parent=nextc->parent;
	}

	bool isleft=false;
	if(nextc->parent==HANDLE_UNDEFINED)
	{
		*root=current;
		parent=HANDLE_UNDEFINED;
	}
	else
	{
		handle nparent=nextc->parent;
		tree_node* nparentc=boot_bitmap_pool_content(pool,nparent);
		if(next==nparentc->left)
		{
			nparentc->left=current;
			isleft=true;
		}
		else
		{
			nparentc->right=current;
			isleft=false;
		}
	}

	uint8 ncolor=nextc->color;

	if(next!=node)
	{
		boot_tree_swap(pool,root,node,next);
		if(parent==node)
		{
			parent=next;
		}
	}

	if(!ncolor)
	{
		boot_tree_remove_rebalance(pool,root,parent,current,isleft);
	}

	nodec->parent=HANDLE_UNDEFINED;
	nodec->left=HANDLE_UNDEFINED;
	nodec->right=HANDLE_UNDEFINED;
}

/*
 * 红黑树结点查找。
 *
 * @param pool	  位映射池。
 * @param root	  树根结点。
 * @param compare 比较函数。
 * @param key	  查找键。
 *
 * @return 返回对应句柄，或未定义。
 */
extern handle boot_tree_find(bitmap_pool* restrict pool,handle* restrict root,const boot_tree_compare compare,const handle key)
{
	handle current=*root;

	while(current!=HANDLE_UNDEFINED)
	{
		tree_node* currentc=boot_bitmap_pool_content(pool,current);
		int comp=compare((handle)currentc,key);

		if(comp==0)
		{
			return current;
		}
		else if(comp>0)
		{
			current=currentc->left;
		}
		else
		{
			current=currentc->right;
		}
	}
	return HANDLE_UNDEFINED;
}

/*
 * 红黑树深度计算。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 *
 * @return 返回该树高度。
 */
extern uintn boot_tree_depth(bitmap_pool* restrict pool,const handle root)
{
	if(root==HANDLE_UNDEFINED)
	{
		return 0;
	}
	else
	{
		tree_node* rootc=boot_bitmap_pool_content(pool,root);

		uintn leftd=boot_tree_depth(pool,rootc->left);
		uintn rightd=boot_tree_depth(pool,rootc->right);
		
		return leftd>rightd?leftd+1:rightd+1;
	}
}

/*
 * 红黑树获取下一个结点。结点需要使用者保证在树上。
 *
 * @param pool 位映射池。
 * @param node 结点句柄。
 *
 * @return 返回该结点的后继结点。
 */
extern handle boot_tree_next(bitmap_pool* restrict pool,const handle node)
{
	if(node==HANDLE_UNDEFINED)
	{
		return HANDLE_UNDEFINED;
	}
	return boot_tree_find_successor(pool,node);
}

/*
 * 红黑树获取上一个结点。结点需要使用者保证在树上。
 *
 * @param pool 位映射池。
 * @param node 结点句柄。
 *
 * @return 返回该结点的前驱结点。
 */
extern handle boot_tree_previous(bitmap_pool* restrict pool,const handle node)
{
	if(node==HANDLE_UNDEFINED)
	{
		return HANDLE_UNDEFINED;
	}
	return boot_tree_find_predecessor(pool,node);
}

/*
 * 红黑树获取头结点。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 *
 * @return 返回该树的最小结点。
 */
extern handle boot_tree_head(bitmap_pool* restrict pool,const handle root)
{
	if(root==HANDLE_UNDEFINED)
	{
		return HANDLE_UNDEFINED;
	}
	return boot_tree_find_minimum(pool,root);
}

/*
 * 红黑树获取尾结点。
 *
 * @param pool 位映射池。
 * @param root 树根结点。
 *
 * @return 返回该树的最大结点。
 */
extern handle boot_tree_tail(bitmap_pool* restrict pool,const handle root)
{
	if(root==HANDLE_UNDEFINED)
	{
		return HANDLE_UNDEFINED;
	}
	return boot_tree_find_maximum(pool,root);
}