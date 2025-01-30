/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.util.rbtree;

import java.util.Random;

/**
 * 红黑树学习。
 * 
 * <p>在2024-12-30时生成。
 *
 * @author Tony Chen Smith
 */
public class RBTree
{
	private Node root=null;
	
	static final class Node
	{
		Node left,right,parent;
		boolean color; //真为红 假为黑
		int key;
		
		Node(int key)
		{
			left=null;
			right=null;
			color=true;
			this.key=key;
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append('[');
			if(left!=null)
			{
				result.append(left).append(',');
			}
			result.append(color?"(R,":"(B,");
			result.append(key);
			result.append(')');
			if(right!=null)
			{
				result.append(',').append(right);
			}
			return result.append(']').toString();
		}
		
		int compare(int key)
		{
			return this.key-key;
		}
	}
	
	/**
	 * 查找。
	 * 
	 * @param key
	 * @return
	 */
	public Node find(int key)
	{
		Node current=root;
		while(current!=null)
		{
			if(current.key==key)
			{
				return current;
			}
			else if(current.key<key)
			{
				current=current.right;
			}
			else
			{
				current=current.left;
			}
		}
		return null;
	}
	
	/**
	 * 获取兄弟
	 * @param node
	 * @return
	 */
	private Node getSibling(Node node)
	{
		if(node.parent==null)
		{
			return null;
		}
		else
		{
			return node.parent.left==node?node.parent.right:node.parent.left;
		}
	}
	
	/**
	 * 获取祖宗
	 * @param node
	 * @return
	 */
	private Node getGrandparent(Node node)
	{
		return node.parent==null?null:node.parent.parent;
	}
	
	/**
	 * 获取叔叔
	 * @param node
	 * @return
	 */
	private Node getUncle(Node node)
	{
		return node.parent==null?null:(node.parent.parent.left==node.parent?node.parent.parent.right:node.parent.parent.left);
	}
	
	/**
	 * 左旋
	 * @param node
	 * @return
	 */
	private void rotateLeft(Node node)
	{
		Node right=node.right;
		node.right=right.left;
		if(right.left!=null)
		{
			right.left.parent=node;
		}
		right.parent=node.parent;
		if(node.parent!=null)
		{
			if(node==node.parent.left)
			{
				node.parent.left=right;
			}
			else
			{
				node.parent.right=right;
			}
		}
		else
		{
			root=right;
		}
		right.left=node;
		node.parent=right;
	}
	
	/**
	 * 右旋
	 * @param node
	 * @return
	 */
	private void rotateRight(Node node)
	{
		Node left=node.left;
		node.left=left.right;
		if(left.right!=null)
		{
			left.right.parent=node;
		}
		left.parent=node.parent;
		if(node.parent!=null)
		{
			if(node==node.parent.right)
			{
				node.parent.right=left;
			}
			else
			{
				node.parent.left=left;
			}
		}
		else
		{
			root=left;
		}
		left.right=node;
		node.parent=left;
	}
	
	/**
	 * 插入后重新平衡
	 * @param node
	 */
	private void iRebalance(Node node)
	{
		Node nparent=node.parent;
		if(nparent!=null&&nparent.color)
		{
			Node current=node;
			while(current!=root&&current!=null&&current.parent!=null&&current.parent.color)
			{
				Node parent=current.parent;
				Node grandparent=getGrandparent(current);
				Node uncle=null;
				boolean uncle_is_left;
				
				if(parent==grandparent.left)
				{
					uncle_is_left=false;
					uncle=grandparent.right;
				}
				else
				{
					uncle_is_left=true;
					uncle=grandparent.left;
				}
				
				if(uncle!=null&&uncle.color)
				{
					//情况1：红叔叔
					parent.color=false;
					uncle.color=false;
					grandparent.color=true;
					current=grandparent;
				}
				else
				{
					//情况2：黑叔叔红家长
					if(!uncle_is_left&&parent.right==current)
					{
						current=current.parent;
						rotateLeft(current);
					}
					else if(uncle_is_left&&parent.left==current)
					{
						current=current.parent;
						rotateRight(current);
					}
					
					//情况3：重新染色和旋转
					parent=current.parent;
					parent.color=false;
					grandparent=getGrandparent(current);
					grandparent.color=true;
					if(uncle_is_left)
					{
						rotateLeft(grandparent);
					}
					else
					{
						rotateRight(grandparent);
					}
				}
			}
			root.color=false;
		}
	}
	
	/**
	 * 插入。
	 * 
	 * @param key
	 * @return
	 */
	public boolean insert(int key)
	{
		Node node=new Node(key);
		
		//情况1：空树
		if(root==null)
		{
			root=node;
			root.color=false;
			return true;
		}
		
		Node current=root;
		while(current!=null)
		{
			int compare=current.compare(key);
			if(compare==0)
			{
				return false;
			}
			if(compare>0)
			{
				if(current.left==null)
				{
					current.left=node;
					break;
				}
				else
				{
					current=current.left;
				}
			}
			else
			{
				if(current.right==null)
				{
					current.right=node;
					break;
				}
				else
				{
					current=current.right;
				}
			}
		}
		node.parent=current;
		iRebalance(node);
		return true;
	}
	
	@Override
	public String toString()
	{
		return root==null?"null":root.toString();
	}
	
	/**
	 * 找最小。
	 * @param node
	 * @return
	 */
	private Node findMinimum(Node node)
	{
		while(node.left!=null)
		{
			node=node.left;
		}
		return node;
	}
	
	/**
	 * 找最大。
	 * @param node
	 * @return
	 */
	private Node findMaximum(Node node)
	{
		while(node.right!=null)
		{
			node=node.right;
		}
		return node;
	}
	
	/**
	 * 找后继
	 * @param node
	 * @return
	 */
	private Node findSuccessor(Node node)
	{
		if(node.right!=null)
		{
			return findMinimum(node.right);
		}
		Node parent=node.parent;
		while(parent!=null&&node==parent.right)
		{
			node=parent;
			parent=parent.parent;
		}
		return parent;
	}
	
	/**
	 * 找前驱
	 * @param node
	 * @return
	 */
	private Node findPredecessor(Node node)
	{
		if(node.left!=null)
		{
			return findMaximum(node.left);
		}
		Node parent=node.parent;
		while(parent!=null&&node==parent.left)
		{
			node=parent;
			parent=parent.parent;
		}
		return parent;
	}
	
	/**
	 * 交换结点，将b置入a的位置
	 * @param a
	 * @param b
	 */
	private void swap(Node a,Node b)
	{
		Node left=a.left;
		Node right=a.right;
		Node parent=a.parent;
		b.parent=parent;
		if(parent!=null)
		{
			if(parent.left==a)
			{
				parent.left=b;
			}
			else
			{
				parent.right=b;
			}
		}
		else
		{
			if(root==a)
			{
				root=b;
			}
		}
		
		b.right=right;
		if(right!=null)
		{
			right.parent=b;
		}
		a.right=null;
		b.left=left;
		if(left!=null)
		{
			left.parent=b;
		}
		a.left=null;
		b.color=a.color;
		a.parent=null;
	}
	
	/**
	 * 删除重平衡
	 * @param parent
	 * @param node
	 * @param node_is_left
	 */
	private void dRebalance(Node parent,Node node,boolean node_is_left)
	{
		//非根黑结点
		while(node!=root&&(node==null||!node.color))
		{
			Node sibling=node_is_left?parent.right:parent.left;
			if(sibling!=null&&sibling.color)
			{
				//情况1：红兄弟
				sibling.color=false;
				parent.color=true;
				if(node_is_left)
				{
					rotateLeft(parent);
				}
				else
				{
					rotateRight(parent);
				}
				sibling=node_is_left?parent.right:parent.left;
			}
			Node sleft=sibling!=null?sibling.left:null;
			Node sright=sibling!=null?sibling.right:null;
			if((sleft==null||!sleft.color)&&(sright==null||!sright.color))
			{
				//情况2：兄弟双黑结点
				if(sibling!=null)
				{
					sibling.color=true;
				}
				node=parent;
				parent=parent.parent;
				node_is_left=parent!=null&&parent.left==node;
			}
			else
			{
				if(node_is_left&&(sright==null||!sright.color))
				{
					//情况3a：右兄弟左红右黑
					sibling.color=true;
					if(sleft!=null)
					{
						sleft.color=false;
					}
					rotateRight(sibling);
					sibling=parent.right;
				}
				else if(!node_is_left&&(sleft==null||!sleft.color))
				{
					//情况3b：左兄弟左黑右红
					sibling.color=true;
					if(sright!=null)
					{
						sright.color=false;
					}
					rotateLeft(sibling);
					sibling=parent.left;
				}
				
				//情况4
				sleft=sibling.left;
				sright=sibling.right;
				sibling.color=parent.color;
				parent.color=false;
				if(node_is_left&&sright!=null)
				{
					sright.color=false;
					rotateLeft(parent);
				}
				else if(!node_is_left&&sleft!=null)
				{
					sleft.color=false;
					rotateRight(parent);
				}
				node=root;
			}
		}
		
		if(node!=null)
		{
			node.color=false;
		}
	}
	
	/**
	 * 删除
	 * @param node
	 */
	public void remove(Node node)
	{
		Node next;
		if(node.left==null||node.right==null)
		{
			//叶子或者单边红
			next=node;
		}
		else
		{
			next=findSuccessor(node);
		}
		
		Node current,parent;
		if(next.left!=null)
		{
			current=next.left;
		}
		else
		{
			current=next.right;
		}
		
		if(current!=null)
		{
			current.parent=next.parent;
			parent=current.parent;
		}
		else
		{
			parent=next.parent;
		}
		
		boolean is_left=false;
		if(next.parent==null)
		{
			root=current;
			parent=null;
		}
		else
		{
			Node nparent=next.parent;
			if(next==nparent.left)
			{
				nparent.left=current;
				is_left=true;
			}
			else
			{
				nparent.right=current;
				is_left=false;
			}
		}
		
		boolean ncolor=next.color;
		if(next!=node)
		{
			swap(node,next);
			if(parent==node)
			{
				parent=next;
			}
		}
		
		if(!ncolor)
		{
			dRebalance(parent,current,is_left);
		}
		
		node.parent=null;
		node.left=null;
		node.right=null;
	}
	
	/**
	 * 主方法。
	 * 
	 * @param args 参数数组。
	 */
	public static void main(String[] args)
	{
		int[] test=new int[]{18,25,15,6,13,37,27,17,34,9,10,23};
		int[] remove=new int[]{18,25,15,6,13,37,27,17,34,9,10,23};
		RBTree tree=new RBTree();
		for(int value:test)
		{
			tree.insert(value);
			//System.out.println(tree);
		}
		for(int value:remove)
		{
			Node removed=tree.find(value);
			tree.remove(removed);
			System.out.println(tree);
		}
	}
	
}
