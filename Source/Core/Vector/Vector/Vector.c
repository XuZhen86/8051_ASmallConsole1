#include"VectorConfig.h"
#include"VectorStatic.h"
#include<Debug.h>
#include<Vector.h>
#include<stdlib.h>
#include<string.h>

Vector *Vector_new(){
    Vector *v=malloc(sizeof(Vector));

    v->elementData=calloc(EXPAND_DEFAULT_SIZE,sizeof(void *));
    v->elementCount=0;
    v->capacity=EXPAND_DEFAULT_SIZE;

    return v;
}

void Vector_delete(Vector *v){
    free(v->elementData);
    free(v);
}

bit Vector_add(Vector *v,void *element){
    if(!Vector_ensureCapacity(v,v->elementCount+1)){
        return 0;
    }

    v->elementData[v->elementCount++]=element;

    return 1;
}

bit Vector_insert(Vector *v,unsigned int index,void *element){
    unsigned int i;

    if(!Vector_ensureCapacity(v,v->elementCount+1)){
        return 0;
    }

    for(i=v->elementCount++;i>index;i--){
        v->elementData[i]=v->elementData[i-1];
    }
    v->elementData[index]=element;

    return 1;
}

unsigned int Vector_capacity(Vector *v){
    return v->capacity;
}

unsigned int Vector_size(Vector *v){
    return v->elementCount;
}

bit Vector_setSize(Vector *v,unsigned int newSize){
    if(newSize<v->elementCount){
        Debug(WARNING,"Truncate Vector@%p size from %u to %u",v,v->elementCount,newSize);
        v->elementCount=newSize;
        return 1;
    }

    if(!Vector_ensureCapacity(v,newSize)){
        return 0;
    }

    memset(v->elementData+v->elementCount*sizeof(void *),0x00,(newSize-v->elementCount)*sizeof(void *));
    v->elementCount=newSize;

    return 1;
}

bit Vector_ensureCapacity(Vector *v,unsigned int minCapacity){
    if(minCapacity>v->capacity){
        if(!expandN(v,minCapacity-v->capacity)){
            return 0;
        }
    }

    return 1;
}

void *Vector_get(Vector *v,unsigned int index){
    if(index>=v->elementCount){
        return NULL;
    }

    return v->elementData[index];
}

void *Vector_set(Vector *v,unsigned int index,void *element){
    void *prevElement;

    if(index>=v->elementCount){
        return NULL;
    }

    prevElement=v->elementData[index];
    v->elementData[index]=element;
    return prevElement;
}

// Only the records will be removed, pointers are not freed
void Vector_removeAllElements(Vector *v){
    v->elementCount=0;
}

bit Vector_removeElementAt(Vector *v,unsigned int index){
    unsigned int i;

    if(index>=v->elementCount){
        return 0;
    }

    v->elementCount--;
    for(i=index;i<v->elementCount;i++){
        v->elementData[i]=v->elementData[i+1];
    }

    return 1;
}

bit Vector_removeElement(Vector *v,void *element){
    unsigned int i;

    for(i=0;i<v->elementCount;i++){
        if(v->elementData[i]==element){
            break;
        }
    }

    if(i==v->elementCount){
        return 0;
    }

    v->elementCount--;
    for(;i<v->elementCount;i++){
        v->elementData[i]=v->elementData[i+1];
    }

    return 1;
}

bit Vector_removeRange(Vector *v,unsigned int fromIndex,unsigned int toIndex){
    unsigned int i,offset;

    if(fromIndex>=toIndex||fromIndex>=v->elementCount||toIndex>=v->elementCount){
        return 0;
    }

    offset=toIndex-fromIndex;
    for(i=fromIndex;i+offset<v->elementCount;i++){
        v->elementData[i]=v->elementData[i+offset];
    }
    v->elementCount-=offset;

    return 1;
}

unsigned int Vector_indexOf(Vector *v,void *element){
    unsigned int i;

    for(i=0;i<v->elementCount;i++){
        if(v->elementData[i]==element){
            break;
        }
    }

    return i;
}

unsigned int Vector_lastIndexOf(Vector *v,void *element){
    unsigned int i;

    for(i=v->elementCount-1;i;i--){
        if(v->elementData[i]==element){
            break;
        }
    }

    if(i==0&&v->elementData[0]!=element){
        return v->elementCount;
    }
    return i;
}

bit Vector_contains(Vector *v,void *element){
    unsigned int i;

    for(i=0;i<v->elementCount;i++){
        if(v->elementData[i]==element){
            return 1;
        }
    }

    return 0;
}

void Vector_debug(Vector *v){
    unsigned int i;

    Debug(DEBUG,"Vector@%p elementCount=%u capacity=%u",v,Vector_size(v),Vector_capacity(v));
    for(i=0;i<v->elementCount;i++){
        Debug(DEBUG,"    elementData[%u]=%p",i,Vector_get(v,i));
    }
}

static bit expandDefault(Vector *v){
    return expandN(v,EXPAND_DEFAULT_SIZE);
}

static bit expandN(Vector *v,unsigned int n){
    void *newElementData;

    newElementData=malloc((v->capacity+n)*sizeof(void *));
    if(newElementData==NULL){
        Debug(WARNING,"Failed to expand Vector@%p by %u",v,n);
        return 0;
    }

    memcpy(newElementData,v->elementData,v->capacity*sizeof(void *));
    free(v->elementData);
    v->elementData=newElementData;
    v->capacity+=n;

    return 1;
}
