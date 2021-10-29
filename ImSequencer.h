#pragma once

#include <cstddef>

struct ImDrawList;
struct ImRect;
namespace sequencer_key_type
{
   enum type
   {
      key,
      range,

      count,
   };
}

struct sequencer_key_data
{
   int Start; 
   int End;
   int Type; 
   unsigned int Color; 
   sequencer_key_type::type KeyType;
};

struct sequencer_track_data
{
   int Start; 
   int End; 
   int Type; 
   unsigned int Color;
};

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
      virtual int GetKeyCount(int trackIdx) const = 0;
      virtual int GetTrackCount() const = 0;

      virtual void BeginEdit(int trackIdx, int frameIdx) {}
      virtual void EndEdit(int trackIdx, int frameIdx) {}
      virtual int GetKeyTypeCount() const { return 0; }
      virtual int GetTrackTypeCount() const { return 0; }
      virtual const char* GetKeyTypeName(int /*typeIndex*/) const { return ""; }
      virtual const char* GetKeyLabel(int trackIdx, int itemIdx) const { return ""; }

      virtual const char* GetTrackTypeName(int /*typeIndex*/) const { return ""; }
      virtual const char* GetTrackLabel(int trackIdx) const { return ""; }

      virtual sequencer_key_data GetKeyData(int trackIdx, int index) = 0;
      virtual sequencer_track_data GetTrackData(int trackIdx) = 0;
      virtual int AddTrack(int trackType) { return -1; }
      virtual void DeleteTrack(int trackIdx) {}
      virtual void DuplicateTrack(int trackIdx) {}
      virtual int MoveTrackUp(int trackIdx) = 0;
      virtual int MoveTrackDown(int trackIdx) = 0;
      virtual int AddKey(int trackIdx, int start, int end) { return -1; }
      virtual int DuplicateKey(int trackIdx, int frameIdx) { return -1; }
      virtual void DeleteKey(int trackIdx, int frameIdx) { }
      virtual i32 MoveKey(int trackIdx, int frameIdx, int newStart, int newEnd) { return -1; }

      virtual void SetTrackActive(int inTrackIdx, bool inIsActive) { }
      virtual bool IsTrackActive(int trackIdx) const { return true; }

      virtual void Copy() {}
      virtual void Paste() {}

      virtual size_t GetCustomTrackHeight(int trackIdx) { return 0; }
      virtual void DoubleClickKey(int trackIdx, int itemIdx) {}
      virtual void DoubleClickTrack(int trackIdx) {}
      virtual void CustomDraw(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*legendRect*/, const ImRect& /*clippingRect*/, const ImRect& /*legendClippingRect*/) {}
      virtual void CustomDrawCompact(int /*index*/, ImDrawList* /*draw_list*/, const ImRect& /*rc*/, const ImRect& /*clippingRect*/) {}

   };


   // return true if selection is made
   bool Sequencer(SequenceInterface* sequence, int* currentFrame, bool* expanded, int* selectedTrack, int* selectedFrame, int* firstFrame, int sequenceOptions);

}
