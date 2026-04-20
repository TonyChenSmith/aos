/**
 * 内核内存池分配器。
 * @date 2026-03-12
 * 
 * Copyright (c) 2026 Tony Chen Smith
 * 
 * SPDX-License-Identifier: MIT
 */
#include <support/memory.h>
#include <support/util.h>

/**
 * 内存池空闲链表结点最小值。
 */
const uintn POOL_FREE_MIN[][8]={
    {BIT6,BIT6+(BIT6>>3),BIT6+2*(BIT6>>3),BIT6+3*(BIT6>>3),BIT6+4*(BIT6>>3),
        BIT6+5*(BIT6>>3),BIT6+6*(BIT6>>3),BIT6+7*(BIT6>>3)},
    {BIT7,BIT7+(BIT7>>3),BIT7+2*(BIT7>>3),BIT7+3*(BIT7>>3),BIT7+4*(BIT7>>3),
        BIT7+5*(BIT7>>3),BIT7+6*(BIT7>>3),BIT7+7*(BIT7>>3)},
    {BIT8,BIT8+(BIT8>>3),BIT8+2*(BIT8>>3),BIT8+3*(BIT8>>3),BIT8+4*(BIT8>>3),
        BIT8+5*(BIT8>>3),BIT8+6*(BIT8>>3),BIT8+7*(BIT8>>3)},
    {BIT9,BIT9+(BIT9>>3),BIT9+2*(BIT9>>3),BIT9+3*(BIT9>>3),BIT9+4*(BIT9>>3),
        BIT9+5*(BIT9>>3),BIT9+6*(BIT9>>3),BIT9+7*(BIT9>>3)},
    {BIT10,BIT10+(BIT10>>3),BIT10+2*(BIT10>>3),BIT10+3*(BIT10>>3),BIT10+4*(BIT10>>3),
        BIT10+5*(BIT10>>3),BIT10+6*(BIT10>>3),BIT10+7*(BIT10>>3)},
    {BIT11,BIT11+(BIT11>>3),BIT11+2*(BIT11>>3),BIT11+3*(BIT11>>3),BIT11+4*(BIT11>>3),
        BIT11+5*(BIT11>>3),BIT11+6*(BIT11>>3),BIT11+7*(BIT11>>3)},
    {BIT12,BIT12+(BIT12>>3),BIT12+2*(BIT12>>3),BIT12+3*(BIT12>>3),BIT12+4*(BIT12>>3),
        BIT12+5*(BIT12>>3),BIT12+6*(BIT12>>3),BIT12+7*(BIT12>>3)},
    {BIT13,BIT13+(BIT13>>3),BIT13+2*(BIT13>>3),BIT13+3*(BIT13>>3),BIT13+4*(BIT13>>3),
        BIT13+5*(BIT13>>3),BIT13+6*(BIT13>>3),BIT13+7*(BIT13>>3)},
    {BIT14,BIT14+(BIT14>>3),BIT14+2*(BIT14>>3),BIT14+3*(BIT14>>3),BIT14+4*(BIT14>>3),
        BIT14+5*(BIT14>>3),BIT14+6*(BIT14>>3),BIT14+7*(BIT14>>3)},
    {BIT15,BIT15+(BIT15>>3),BIT15+2*(BIT15>>3),BIT15+3*(BIT15>>3),BIT15+4*(BIT15>>3),
        BIT15+5*(BIT15>>3),BIT15+6*(BIT15>>3),BIT15+7*(BIT15>>3)},
    {BIT16,BIT16+(BIT16>>3),BIT16+2*(BIT16>>3),BIT16+3*(BIT16>>3),BIT16+4*(BIT16>>3),
        BIT16+5*(BIT16>>3),BIT16+6*(BIT16>>3),BIT16+7*(BIT16>>3)},
    {BIT17,BIT17+(BIT17>>3),BIT17+2*(BIT17>>3),BIT17+3*(BIT17>>3),BIT17+4*(BIT17>>3),
        BIT17+5*(BIT17>>3),BIT17+6*(BIT17>>3),BIT17+7*(BIT17>>3)},
    {BIT18,BIT18+(BIT18>>3),BIT18+2*(BIT18>>3),BIT18+3*(BIT18>>3),BIT18+4*(BIT18>>3),
        BIT18+5*(BIT18>>3),BIT18+6*(BIT18>>3),BIT18+7*(BIT18>>3)},
    {BIT19,BIT19+(BIT19>>3),BIT19+2*(BIT19>>3),BIT19+3*(BIT19>>3),BIT19+4*(BIT19>>3),
        BIT19+5*(BIT19>>3),BIT19+6*(BIT19>>3),BIT19+7*(BIT19>>3)},
    {BIT20,BIT20+(BIT20>>3),BIT20+2*(BIT20>>3),BIT20+3*(BIT20>>3),BIT20+4*(BIT20>>3),
        BIT20+5*(BIT20>>3),BIT20+6*(BIT20>>3),BIT20+7*(BIT20>>3)},
    {BIT21,BIT21+(BIT21>>3),BIT21+2*(BIT21>>3),BIT21+3*(BIT21>>3),BIT21+4*(BIT21>>3),
        BIT21+5*(BIT21>>3),BIT21+6*(BIT21>>3),BIT21+7*(BIT21>>3)},
    {BIT22,BIT22+(BIT22>>3),BIT22+2*(BIT22>>3),BIT22+3*(BIT22>>3),BIT22+4*(BIT22>>3),
        BIT22+5*(BIT22>>3),BIT22+6*(BIT22>>3),BIT22+7*(BIT22>>3)},
    {BIT23,BIT23+(BIT23>>3),BIT23+2*(BIT23>>3),BIT23+3*(BIT23>>3),BIT23+4*(BIT23>>3),
        BIT23+5*(BIT23>>3),BIT23+6*(BIT23>>3),BIT23+7*(BIT23>>3)},
    {BIT24,BIT24+(BIT24>>3),BIT24+2*(BIT24>>3),BIT24+3*(BIT24>>3),BIT24+4*(BIT24>>3),
        BIT24+5*(BIT24>>3),BIT24+6*(BIT24>>3),BIT24+7*(BIT24>>3)},
    {BIT25,BIT25+(BIT25>>3),BIT25+2*(BIT25>>3),BIT25+3*(BIT25>>3),BIT25+4*(BIT25>>3),
        BIT25+5*(BIT25>>3),BIT25+6*(BIT25>>3),BIT25+7*(BIT25>>3)},
    {BIT26,BIT26+(BIT26>>3),BIT26+2*(BIT26>>3),BIT26+3*(BIT26>>3),BIT26+4*(BIT26>>3),
        BIT26+5*(BIT26>>3),BIT26+6*(BIT26>>3),BIT26+7*(BIT26>>3)},
    {BIT27,BIT27+(BIT27>>3),BIT27+2*(BIT27>>3),BIT27+3*(BIT27>>3),BIT27+4*(BIT27>>3),
        BIT27+5*(BIT27>>3),BIT27+6*(BIT27>>3),BIT27+7*(BIT27>>3)},
    {BIT28,BIT28+(BIT28>>3),BIT28+2*(BIT28>>3),BIT28+3*(BIT28>>3),BIT28+4*(BIT28>>3),
        BIT28+5*(BIT28>>3),BIT28+6*(BIT28>>3),BIT28+7*(BIT28>>3)},
    {BIT29,BIT29+(BIT29>>3),BIT29+2*(BIT29>>3),BIT29+3*(BIT29>>3),BIT29+4*(BIT29>>3),
        BIT29+5*(BIT29>>3),BIT29+6*(BIT29>>3),BIT29+7*(BIT29>>3)},
    {BIT30,BIT30+(BIT30>>3),BIT30+2*(BIT30>>3),BIT30+3*(BIT30>>3),BIT30+4*(BIT30>>3),
        BIT30+5*(BIT30>>3),BIT30+6*(BIT30>>3),BIT30+7*(BIT30>>3)},
    {BIT31,BIT31+(BIT31>>3),BIT31+2*(BIT31>>3),BIT31+3*(BIT31>>3),BIT31+4*(BIT31>>3),
        BIT31+5*(BIT31>>3),BIT31+6*(BIT31>>3),BIT31+7*(BIT31>>3)},
    {BIT32,BIT32+(BIT32>>3),BIT32+2*(BIT32>>3),BIT32+3*(BIT32>>3),BIT32+4*(BIT32>>3),
        BIT32+5*(BIT32>>3),BIT32+6*(BIT32>>3),BIT32+7*(BIT32>>3)},
    {BIT33,BIT33+(BIT33>>3),BIT33+2*(BIT33>>3),BIT33+3*(BIT33>>3),BIT33+4*(BIT33>>3),
        BIT33+5*(BIT33>>3),BIT33+6*(BIT33>>3),BIT33+7*(BIT33>>3)},
    {BIT34,BIT34+(BIT34>>3),BIT34+2*(BIT34>>3),BIT34+3*(BIT34>>3),BIT34+4*(BIT34>>3),
        BIT34+5*(BIT34>>3),BIT34+6*(BIT34>>3),BIT34+7*(BIT34>>3)},
    {BIT35,BIT35+(BIT35>>3),BIT35+2*(BIT35>>3),BIT35+3*(BIT35>>3),BIT35+4*(BIT35>>3),
        BIT35+5*(BIT35>>3),BIT35+6*(BIT35>>3),BIT35+7*(BIT35>>3)},
    {BIT36,BIT36+(BIT36>>3),BIT36+2*(BIT36>>3),BIT36+3*(BIT36>>3),BIT36+4*(BIT36>>3),
        BIT36+5*(BIT36>>3),BIT36+6*(BIT36>>3),BIT36+7*(BIT36>>3)},
    {BIT37,BIT37+(BIT37>>3),BIT37+2*(BIT37>>3),BIT37+3*(BIT37>>3),BIT37+4*(BIT37>>3),
        BIT37+5*(BIT37>>3),BIT37+6*(BIT37>>3),BIT37+7*(BIT37>>3)}
};

/**
 * 内存池结点。
 */
typedef struct _pool_tlsf_node pool_tlsf_node;

struct _pool_tlsf_node
{
    uintn           psize; /*前一物理连续结点大小。*/
    uintn           csize; /*当前结点大小。*/
    pool_tlsf_node* prev;  /*链表前一结点。*/
    pool_tlsf_node* next;  /*链表下一结点。*/
};

/**
 * 内存池元数据。
 */
typedef struct _pool_tlsf_meta
{
    uint32          fl_bitmap;        /*第一级位图。*/
    uint8           sl_bitmap[32];    /*第二级位图。*/
    pool_tlsf_node* free_head[32][8]; /*空闲块链表头。*/
    pool_tlsf_node* free_tail[32][8]; /*空闲块链表尾。*/
    pool_tlsf_node* allow_head;       /*已分配块链表头。*/
    pool_tlsf_node* allow_tail;       /*已分配块链表尾。*/
} pool_tlsf_meta;

/**
 * 获取结点大小对应的第一级索引。小于64统一归为0，大于等于128GB的统一归为31。
 * 
 * @param size 结点大小。
 * 
 * @return 对应的第一级索引。
 */
static inline uintn pool_get_fl_index(uintn size)
{
    if(size<64)
    {
        return 0;
    }
    uintn fl=sizeof(uintn)*8-count_leading_zeros(size)-7;
    return fl>31?31:fl;
}

/**
 * 获取结点大小对应的第二级索引。
 * 
 * @param size     结点大小。
 * @param fl_index 对应的第一级索引。
 * 
 * @return 对应的第二级索引。
 */
static inline uintn pool_get_sl_index(uintn size,uintn fl_index)
{
    if(size<64||fl_index>=31)
    {
        return 0;
    }
    return (size>>(fl_index+3))&0x7;
}

/**
 * 获取内存申请大小对应的起始索引组。
 * 
 * @param size     内存申请大小。
 * @param fl_index 对应的第一级索引。
 * @param sl_index 对应的第二级索引。
 * 
 * @return 无返回值。
 */
static inline void pool_get_alloc_index(uintn size,uintn* fl_index,uintn* sl_index)
{
    *fl_index=pool_get_fl_index(size);
    *sl_index=pool_get_sl_index(size,*fl_index);
    if(size>64&&*fl_index<31&&size>POOL_FREE_MIN[*fl_index][*sl_index])
    {
        if(*sl_index==7)
        {
            (*fl_index)++;
            *sl_index=0;
        }
        else
        {
            (*sl_index)++;
        }
    }
}

/**
 * 内存池链表添加结点。
 * 
 * @param head 链表头节点。
 * @param tail 链表尾结点。
 * @param node 待添加结点。
 * 
 * @return 无返回值。
 */
static inline void pool_list_add(pool_tlsf_node** head,pool_tlsf_node** tail,pool_tlsf_node* node)
{
    node->prev=null;
    node->next=*head;
    if(*head==null)
    {
        *tail=node;
    }
    else
    {
        (*head)->prev=node;
    }
    *head=node;
}

/**
 * 内存池链表删除结点。
 * 
 * @param head 链表头节点。
 * @param tail 链表尾结点。
 * @param node 待删除结点。
 * 
 * @return 无返回值。
 */
static inline void pool_list_remove(pool_tlsf_node** head,pool_tlsf_node** tail,pool_tlsf_node* node)
{
    if(node->prev!=null)
    {
        node->prev->next=node->next;
    }
    else
    {
        *head=node->next;
    }
    if(node->next!=null)
    {
        node->next->prev=node->prev;
    }
    else
    {
        *tail=node->prev;
    }
    node->prev=null;
    node->next=null;
}

/**
 * 将大小信息和块空闲状态打包。
 * 
 * @param size 大小信息。
 * @param free 块空闲状态信息。空闲块为真。
 * 
 * @return 打包后数据。
 */
static inline uintn pool_pack_size(uintn size,bool free)
{
    return free?size|BIT0:size&(UINTN_MAX^BIT0);
}

/**
 * 获取打包信息中的大小信息。
 * 
 * @param packed 打包信息。
 * 
 * @return 大小信息。
 */
static inline uintn pool_get_size(uintn packed)
{
    return packed&(UINTN_MAX^BIT0);
}

/**
 * 获取打包信息中的块空闲状态信息。
 * 
 * @param packed 打包信息。
 * 
 * @return 块空闲状态信息。
 */
static inline bool pool_is_free(uintn packed)
{
    return packed&BIT0?true:false;
}

/**
 * 当前内存池元数据。
 */
static pool_tlsf_meta pool_meta;

/**
 * 根据第一级索引和第二级索引更新映射。
 * 
 * @param fl_index 第一级索引。
 * @param sl_index 第二级索引。
 * 
 * @return 无返回值。
 */
static void pool_update_bitmap(uintn fl_index,uintn sl_index)
{
    if(pool_meta.free_head[fl_index][sl_index]!=null)
    {
        pool_meta.fl_bitmap|=1<<fl_index;
        pool_meta.sl_bitmap[fl_index]|=1<<sl_index;
    }
    else
    {
        pool_meta.sl_bitmap[fl_index]&=UINT8_MAX^(1<<sl_index);
        if(pool_meta.sl_bitmap[fl_index]==0)
        {
            pool_meta.fl_bitmap&=UINT32_MAX^(1<<fl_index);
        }
    }
}

/**
 * 当前初始化状态。
 */
static bool init_state=false;

/**
 * 初始化内核内存池。
 * 
 * @param pool  内核内存池基址。
 * @param pages 内存块页数。
 * 
 * @return 成功初始化返回真。
 */
bool memory_pool_init(void* pool,uintn pages)
{
    if(init_state||pool==null||pages<=1)
    {
        return false;
    }
    else
    {
        init_state=true;
    }
    memory_zero(&pool_meta,sizeof(pool_tlsf_meta));

    pool_tlsf_node* block=(pool_tlsf_node*)pool;
    block->psize=pool_pack_size(0,false);
    block->csize=pool_pack_size(
        (pages<<12)-align_up(sizeof(pool_tlsf_meta),8)-sizeof(pool_tlsf_node),true);
    uintn fl_index=pool_get_fl_index(block->csize);
    uintn sl_index=pool_get_sl_index(block->csize,fl_index);
    pool_list_add(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
        block);
    pool_update_bitmap(fl_index,sl_index);

    pool_tlsf_node* sentinel=(pool_tlsf_node*)((uintn)block+pool_get_size(block->csize));
    sentinel->psize=block->csize;
    sentinel->csize=pool_pack_size(sizeof(pool_tlsf_node),false);
    pool_list_add(&pool_meta.allow_head,&pool_meta.allow_tail,sentinel);

    return true;
}

/**
 * 内存池新增页面申请。
 * 
 * @param pages 申请页数。
 * 
 * @return 申请成功返回一个非零线性地址。
 */
static void* (*pool_page_alloc)(uintn pages)=null;

/**
 * 尝试添加一块足够大的页框来申请一块内核内存池的内存。
 * 
 * @param size 经过调整的申请大小。
 * 
 * @return 成功申请返回一个非空指针。
 */
static void* pool_alloc_new(uintn size)
{
    if(pool_page_alloc==null)
    {
        return null;
    }

    /*保证申请块除了能容纳申请内存外还能接受一个哨兵块*/
    uintn block_size=align_up(size+sizeof(pool_tlsf_node),SIZE_4KB);
    pool_tlsf_node* block=(pool_tlsf_node*)pool_page_alloc(block_size>>12);
    if(block==null)
    {
        return null;
    }
    block->psize=pool_pack_size(0,true);
    block_size-=sizeof(pool_tlsf_node);

    pool_tlsf_node* sentinel=(pool_tlsf_node*)((uintn)block+block_size);
    sentinel->csize=pool_pack_size(sizeof(pool_tlsf_node),false);
    pool_list_add(&pool_meta.allow_head,&pool_meta.allow_tail,sentinel);
    pool_list_add(&pool_meta.allow_head,&pool_meta.allow_tail,block);

    uintn space=block_size-size;
    if(space>=64)
    {
        pool_tlsf_node* free_block=(pool_tlsf_node*)((uintn)block+size);
        block->csize=pool_pack_size(size,false);
        free_block->psize=block->csize;
        free_block->csize=pool_pack_size(space,true);
        sentinel->psize=free_block->csize;

        uintn fl_index=pool_get_fl_index(space);
        uintn sl_index=pool_get_sl_index(space,fl_index);
        pool_list_add(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
            free_block);
        pool_update_bitmap(fl_index,sl_index);
    }
    else
    {
        block->csize=pool_pack_size(block_size,false);
        sentinel->psize=block->csize;
    }

    return (void*)((uintn)block+sizeof(pool_tlsf_node));
}

/**
 * 申请一块内核内存池的内存。
 * 
 * @param size 申请大小。
 * 
 * @return 成功申请返回一个非空指针。
 */
void* pool_alloc(uintn size)
{
    if(size==0)
    {
        return null;
    }

    size=max(align_up(sizeof(pool_tlsf_node)+size,8),64);
    uintn fl_index,sl_index;
    pool_get_alloc_index(size,&fl_index,&sl_index);

    pool_tlsf_node* block=null;
    if(fl_index==31)
    {
        pool_tlsf_node* node=pool_meta.free_head[31][0];
        while(node!=null)
        {
            if(pool_get_size(node->csize)>=size)
            {
                block=node;
                break;
            }
            node=node->next;
        }
        if(block==null)
        {
            return pool_alloc_new(size);
        }
    }
    else
    {
        uintn free_fl=count_trailing_zeros(pool_meta.fl_bitmap&(UINT32_MAX<<fl_index));
        if(free_fl>31)
        {
            return pool_alloc_new(size);
        }
        else if(free_fl==fl_index)
        {
            uintn free_sl=count_trailing_zeros((uint8)(pool_meta.sl_bitmap[fl_index]&(UINT8_MAX<<sl_index)));
            if(free_sl>7)
            {
                free_fl=count_trailing_zeros(pool_meta.fl_bitmap&(UINT32_MAX<<(fl_index+1)));
                if(free_fl>31)
                {
                    return pool_alloc_new(size);
                }
                else
                {
                    fl_index=free_fl;
                    sl_index=count_trailing_zeros(pool_meta.sl_bitmap[fl_index]);
                }
            }
            else
            {
                sl_index=free_sl;
            }
        }
        else
        {
            fl_index=free_fl;
            sl_index=count_trailing_zeros(pool_meta.sl_bitmap[fl_index]);
        }
    
        block=pool_meta.free_head[fl_index][sl_index];
    }
    
    pool_list_remove(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
        block);
    pool_update_bitmap(fl_index,sl_index);
    pool_list_add(&pool_meta.allow_head,&pool_meta.allow_tail,block);

    uintn space=pool_get_size(block->csize)-size;
    if(space>=64)
    {
        pool_tlsf_node* free_block=(pool_tlsf_node*)((uintn)block+size);
        block->csize=pool_pack_size(size,false);
        free_block->psize=block->csize;
        free_block->csize=pool_pack_size(space,true);
        pool_tlsf_node* next=(pool_tlsf_node*)((uintn)free_block+space);
        next->psize=free_block->csize;

        fl_index=pool_get_fl_index(space);
        sl_index=pool_get_sl_index(space,fl_index);
        pool_list_add(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
            free_block);
        pool_update_bitmap(fl_index,sl_index);
    }
    else
    {
        block->csize=pool_pack_size(block->csize,false);
    }

    return (void*)((uintn)block+sizeof(pool_tlsf_node));
}

/**
 * 内存池释放申请页面。
 * 
 * @param base  释放页面线性基址。
 * @param pages 释放页数。
 * 
 * @return 无返回值。
 */
static void (*pool_page_free)(void* base,uintn pages)=null;

/**
 * 释放一块内核内存池的内存。
 * 
 * @param ptr 申请内存指针基址。
 * 
 * @return 无返回值。
 */
void pool_free(void* ptr)
{
    if(ptr==null||(uintn)ptr%8!=0)
    {
        return;
    }
    pool_tlsf_node* block=(pool_tlsf_node*)((uintn)ptr-sizeof(pool_tlsf_node));
    if(pool_is_free(block->csize)||pool_get_size(block->csize)<64)
    {
        return;
    }
    pool_list_remove(&pool_meta.allow_head,&pool_meta.allow_tail,block);

    if(pool_is_free(block->psize)&&pool_get_size(block->psize)>0)
    {
        pool_tlsf_node* prev=(pool_tlsf_node*)((uintn)block-pool_get_size(block->psize));
        uintn fl_index=pool_get_fl_index(pool_get_size(prev->csize));
        uintn sl_index=pool_get_sl_index(pool_get_size(prev->csize),fl_index);
        pool_list_remove(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
            prev);
        pool_update_bitmap(fl_index,sl_index);

        prev->csize=pool_pack_size(pool_get_size(prev->csize)+pool_get_size(block->csize),
            true);
        block=prev;
    }
    else
    {
        block->csize=pool_pack_size(block->csize,true);
    }

    pool_tlsf_node* next=(pool_tlsf_node*)((uintn)block+pool_get_size(block->csize));
    if(pool_is_free(next->csize))
    {
        uintn fl_index=pool_get_fl_index(pool_get_size(next->csize));
        uintn sl_index=pool_get_sl_index(pool_get_size(next->csize),fl_index);
        pool_list_remove(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
            next);
        pool_update_bitmap(fl_index,sl_index);

        block->csize=pool_pack_size(pool_get_size(block->csize)+pool_get_size(next->csize),
            true);
        next=(pool_tlsf_node*)((uintn)block+pool_get_size(block->csize));
        next->psize=block->csize;
    }
    else
    {
        next->psize=block->csize;
    }

    if(block->psize==pool_pack_size(0,true)&&next->csize==sizeof(pool_tlsf_node))
    {
        uintn size=(pool_get_size(block->csize)+sizeof(pool_tlsf_node));
        pool_list_remove(&pool_meta.allow_head,&pool_meta.allow_tail,next);
        pool_page_free(block,size>>12);
    }
    else
    {
        uintn fl_index=pool_get_fl_index(pool_get_size(block->csize));
        uintn sl_index=pool_get_sl_index(pool_get_size(block->csize),fl_index);
        pool_list_add(&pool_meta.free_head[fl_index][sl_index],&pool_meta.free_tail[fl_index][sl_index],
            block);
        pool_update_bitmap(fl_index,sl_index);
    }
}

/**
 * 内核内存池接入页管理器。
 * 
 * @return 无返回值。
 */
void memory_pool_attach_page_allocator(void)
{

}