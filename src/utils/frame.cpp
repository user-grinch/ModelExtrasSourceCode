#include "pch.h"
#include "frame.h"
#include <rw/rpworld.h>

void FrameUtil::SetRotationX(RwFrame *frame, float angle)
{
    RwFrameRotate(frame, (RwV3d *)0x008D2E00, (RwReal)angle, rwCOMBINEPRECONCAT);
    RwFrameUpdateObjects(frame);
}

void FrameUtil::SetRotationY(RwFrame *frame, float angle)
{
    RwFrameRotate(frame, (RwV3d *)0x008D2E0C, (RwReal)angle, rwCOMBINEPRECONCAT);
    RwFrameUpdateObjects(frame);
}

void FrameUtil::SetRotationZ(RwFrame *frame, float angle)
{
    RwFrameRotate(frame, (RwV3d *)0x008D2E18, (RwReal)angle, rwCOMBINEPRECONCAT);
    RwFrameUpdateObjects(frame);
}

uint32_t FrameUtil::GetChildCount(RwFrame *parent)
{
    RwFrame *child = parent->child;
    uint32_t count = 0U;
    if (child)
    {
        while (child)
        {
            ++count;
            child = child->next;
        }
        return count;
    }
    return 0U;
}

void FrameUtil::StoreChilds(RwFrame *parent, std::vector<RwFrame *> &store)
{
    RwFrame *child = parent->child;

    while (child)
    {
        store.push_back(child);
        child = child->next;
    }
}

void FrameUtil::ShowAllAtomics(RwFrame *frame)
{
    if (!rwLinkListEmpty(&frame->objectList))
    {
        RwObjectHasFrame *atomic;

        RwLLLink *current = rwLinkListGetFirstLLLink(&frame->objectList);
        RwLLLink *end = rwLinkListGetTerminator(&frame->objectList);

        do
        {
            atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
            atomic->object.flags |= rpATOMICRENDER; // clear

            current = rwLLLinkGetNext(current);
        } while (current != end);
    }
}

void FrameUtil::HideAllAtomics(RwFrame *frame)
{
    if (!rwLinkListEmpty(&frame->objectList))
    {
        RwObjectHasFrame *atomic;

        RwLLLink *current = rwLinkListGetFirstLLLink(&frame->objectList);
        RwLLLink *end = rwLinkListGetTerminator(&frame->objectList);

        while (current != end)
        {
            atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
            atomic->object.flags &= ~rpATOMICRENDER;

            current = rwLLLinkGetNext(current);
        }
    }
}

void FrameUtil::HideChildWithName(RwFrame *parent_frame, const char *name)
{
    RwFrame *child = parent_frame->child;
    while (child)
    {
        if (!strcmp(GetFrameNodeName(child), name))
        {
            FrameUtil::HideAllAtomics(child);
            return;
        }
        child = child->next;
    }
}

void FrameUtil::ShowChildWithName(RwFrame *parent_frame, const char *name)
{
    RwFrame *child = parent_frame->child;
    while (child)
    {
        if (!strcmp(GetFrameNodeName(child), name))
        {
            FrameUtil::ShowAllAtomics(child);
            return;
        }
        child = child->next;
    }
}

void FrameUtil::HideAllChilds(RwFrame *parent_frame)
{
    RwFrame *child = parent_frame->child;
    while (child)
    {
        FrameUtil::HideAllAtomics(child);
        child = child->next;
    }
    FrameUtil::HideAllAtomics(parent_frame);
}

void FrameUtil::ShowAllChilds(RwFrame *parent_frame)
{
    RwFrame *child = parent_frame->child;
    while (child)
    {
        FrameUtil::ShowAllAtomics(child);
        child = child->next;
    }
    FrameUtil::ShowAllAtomics(parent_frame);
}