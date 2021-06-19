#pragma once

#include <cstddef>

struct ImDrawList;
struct ImRect;
namespace ImSequencer
{
   enum SEQUENCER_OPTIONS
   {
      SEQUENCER_EDIT_NONE = 0,
      SEQUENCER_EDIT_STARTEND = 1 << 1,
      SEQUENCER_CHANGE_FRAME = 1 << 3,
      SEQUENCER_ADD = 1 << 4,
      SEQUENCER_DEL = 1 << 5,
      SEQUENCER_COPYPASTE = 1 << 6,
      SEQUENCER_EDIT_ALL = SEQUENCER_EDIT_STARTEND | SEQUENCER_CHANGE_FRAME
   };

   struct SequenceInterface
   {
      bool focused = false;
      virtual int GetFrameMin() const = 0;
      virtual int GetFrameMax() const = 0;
      virtual int GetItemCount(int layerIdx) const = 0;
      virtual int GetLayerCount() const = 0;

      virtual void BeginEdit(int /*index*/) {}
      virtual void EndEdit() {}
      virtual int GetItemTypeCount() const { return 0; }
      virtual int GetLayerTypeCount() const { return 0; }
      virtual const char* GetItemTypeName(int /*typeIndex*/) const { return ""; }
      virtual const char* GetItemLabel(int layerIdx, int itemIdx) const { return ""; }

      virtual const char* GetLayerTypeName(int /*typeIndex*/) const { return ""; }
      virtual const char* GetLayerLabel(int layerIdx) const { return ""; }

      virtual void GetFrame(int layerIdx, int index, int** start, int** end, int* type, unsigned int* color) = 0;
      virtual void GetLayer(int layerIdx, int** start, int** end, int* type, unsigned int* color) = 0;
      virtual int AddLayer(int layerType) { return -1; }
      virtual void DeleteLayer(int layerIdx) {}
      virtual void DuplicateLayer(int layerIdx) {}
      virtual int AddFrame(int layerIdx, int start, int end) { return -1; }

      virtual void Copy() {}
      virtual void Paste() {}

      virtual size_t GetCustomLayerHeight(int layerIdx) { return 0; }
      virtual void DoubleClick(int layerIdx, int itemIdx) {}
      virtual void CustomDraw(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*legendRect*/, const ImRect& /*clippingRect*/, const ImRect& /*legendClippingRect*/) {}
      virtual void CustomDrawCompact(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*clippingRect*/) {}
   };


   // return true if selection is made
   bool Sequencer(SequenceInterface* sequence, int* currentFrame, bool* expanded, int* selectedLayer, int* selectedFrame, int* firstFrame, int sequenceOptions);

}
