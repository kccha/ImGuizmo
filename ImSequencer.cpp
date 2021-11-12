#include "ImSequencer.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Utility.h"
#include <cstdlib>
#include <windows.h>

ImVec2 Min(ImVec2 a, ImVec2 b)
{
   return ImVec2(MIN(a.x, b.x), MIN(a.y, b.y));
}

ImVec2 Max(ImVec2 a, ImVec2 b)
{
   return ImVec2(MAX(a.x, b.x), MAX(a.y, b.y));
}


namespace ImSequencer
{
   constexpr int BaseColor = 0xFF777777;
   constexpr int HoverColor = 0xFFAAAAAA;
   static bool SequencerCopyButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
   {
      ImGuiIO& io = ImGui::GetIO();
      ImRect rect(pos, ImVec2(pos.x + 16, pos.y + 16));
      bool isOver = rect.Contains(io.MousePos);
      int color = isOver ? HoverColor : BaseColor;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      draw_list->AddRect(rect.Min, rect.Max, color, 4);
      ImRect copyRect(pos, ImVec2(pos.x + 8, pos.y + 8));
      copyRect.Translate(ImVec2(3, 3));
      draw_list->AddRect(copyRect.Min, copyRect.Max, color, 4);
      copyRect.Translate(ImVec2(2, 2));
      draw_list->AddRect(copyRect.Min, copyRect.Max, color, 4);
      // TODO(KCC): #CopyTrack #NewFrame
      return isOver;
   }

   static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
   {
      ImGuiIO& io = ImGui::GetIO();
      ImRect delRect(pos, ImVec2(pos.x + 16, pos.y + 16));
      bool overDel = delRect.Contains(io.MousePos);
      int delColor = overDel ? HoverColor : BaseColor;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
      draw_list->AddLine(ImVec2(delRect.Min.x + 3, midy), ImVec2(delRect.Max.x - 3, midy), delColor, 2);
      if (add)
         draw_list->AddLine(ImVec2(midx, delRect.Min.y + 3), ImVec2(midx, delRect.Max.y - 3), delColor, 2);

      // TODO(KCC): #CopyTrack #NewFrame
      return overDel;
   }

   static bool SequencerMoveUpButton(ImDrawList* draw_list, ImVec2 pos)
   {
      ImGuiIO& io = ImGui::GetIO();
      ImRect rect(pos, ImVec2(pos.x + 16, pos.y + 16));
      bool overDel = rect.Contains(io.MousePos);
      int delColor = overDel ? HoverColor : BaseColor;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      float topY = rect.Min.y + 3;
      float baseY = rect.Max.y - 4;
      draw_list->AddRect(rect.Min, rect.Max, delColor, 4);
      draw_list->AddLine(ImVec2(rect.Min.x + 3, baseY), ImVec2(midx, topY), delColor, 2);
      draw_list->AddLine(ImVec2(rect.Max.x - 3, baseY), ImVec2(midx, topY), delColor, 2);
      return overDel;
   }

   static bool SequencerMoveDownButton(ImDrawList* draw_list, ImVec2 pos)
   {
      ImGuiIO& io = ImGui::GetIO();
      ImRect rect(pos, ImVec2(pos.x + 16, pos.y + 16));
      bool overDel = rect.Contains(io.MousePos);
      int delColor = overDel ? HoverColor : BaseColor;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      float topY = rect.Min.y + 3;
      float baseY = rect.Max.y - 4;
      draw_list->AddRect(rect.Min, rect.Max, delColor, 4);

      draw_list->AddLine(ImVec2(rect.Min.x + 3, topY), ImVec2(midx, baseY), delColor, 2);
      draw_list->AddLine(ImVec2(rect.Max.x - 3, topY), ImVec2(midx, baseY), delColor, 2);
      return overDel;
   }

   static bool SequencerActiveButton(ImDrawList* draw_list, ImVec2 pos, bool isVisible)
   {
      ImGuiIO& io = ImGui::GetIO();
      ImRect rect(pos, ImVec2(pos.x + 16, pos.y + 16));
      ImRect insideRect(pos + ImVec2(3, 3), ImVec2(pos.x + 13, pos.y + 13));
      bool overDel = rect.Contains(io.MousePos);
      int visibleColor = overDel ? HoverColor : BaseColor;
      float midy = pos.y + 16 / 2 - 0.5f;
      float midx = pos.x + 16 / 2 - 0.5f;
      float topY = rect.Min.y + 3;
      float baseY = rect.Max.y - 4;
      if (isVisible)
      {
         draw_list->AddRectFilled(insideRect.Min, insideRect.Max, visibleColor, 4);
      }
      else
      {
         draw_list->AddRect(insideRect.Min, insideRect.Max, visibleColor, 4);
      }
      return overDel;
   }


   struct next_button_data
   {
      float ButtonX;
      float ButtonY;
      float TrackHeight;
      int NextButtonIdx;
   };

   next_button_data GetNextButtonData(float baseX, float baseY, int buttonIdx, float trackHeight, bool moveLeft)
   {
      next_button_data result = {};
      float multiplier = (moveLeft) ? -1.0f : 1.0f;
      result.ButtonX = baseX + buttonIdx * trackHeight * multiplier;
      result.ButtonY = baseY;
      result.NextButtonIdx = buttonIdx + 1;
      result.TrackHeight = trackHeight;

      return result;
   }

   bool Sequencer(SequenceInterface* sequence, int* currentFrame, bool* expanded, int* selectedTrack, int* selectedKey, int* firstFrame, int sequenceOptions)
   {
      int TrackSelectionBackgroundColor = 0x801080FF;
      bool ret = false;
      ImGuiIO& io = ImGui::GetIO();
      int cx = (int)(io.MousePos.x);
      int cy = (int)(io.MousePos.y);
      static float framePixelWidth = 10.f;
      static float framePixelWidthTarget = 10.f;
      int legendWidth = 400;

      static int movingTrack = -1;
      static int movingKey = -1;
      static int movingPos = -1;
      static int movingPart = -1;
      int delTrackIdx = -1;
      int dupTrackIdx = -1;
      int trackToAddKeyIdx = -1;
      int moveUpTrackIdx = -1;
      int moveDownTrackIdx = -1;
      int TrackHeight = 20;
      int mouseFrame = -1;

      bool popupOpened = false;
      int trackCount = sequence->GetTrackCount();
      ImGui::BeginGroup();

      ImDrawList* draw_list = ImGui::GetWindowDrawList();
      ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
      ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
      int firstFrameUsed = firstFrame ? *firstFrame : 0;


      int controlHeight = trackCount * TrackHeight;
      for (int i = 0; i < trackCount; i++)
      {
         controlHeight += int(sequence->GetCustomTrackHeight(i));
      }
      // This prevents a crash if we have 0 tracks.
      if (controlHeight == 0)
      {
         controlHeight = 1 * TrackHeight; 
      }

      int frameCount = ImMax(sequence->GetFrameMax() - sequence->GetFrameMin(), 1);

      static bool MovingScrollBar = false;
      static bool MovingCurrentFrame = false;
      struct CustomDraw
      {
         int index;
         ImRect customRect;
         ImRect legendRect;
         ImRect clippingRect;
         ImRect legendClippingRect;
      };
      ImVector<CustomDraw> customDraws;
      ImVector<CustomDraw> compactCustomDraws;
      // zoom in/out
      const int visibleFrameCount = (int)floorf((canvas_size.x - legendWidth) / framePixelWidth);
      const float barWidthRatio = ImMin(visibleFrameCount / (float)frameCount, 1.f);
      const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

      ImRect regionRect(canvas_pos, canvas_pos + canvas_size);

      static bool panningView = false;
      static ImVec2 panningViewSource;
      static int panningViewFrame;
      if (ImGui::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
      {
         if (!panningView)
         {
            panningViewSource = io.MousePos;
            panningView = true;
            panningViewFrame = *firstFrame;
         }
         *firstFrame = panningViewFrame - int((io.MousePos.x - panningViewSource.x) / framePixelWidth);
         *firstFrame = ImClamp(*firstFrame, sequence->GetFrameMin(), sequence->GetFrameMax() - visibleFrameCount);
      }
      if (panningView && !io.MouseDown[2])
      {
         panningView = false;
      }
      framePixelWidthTarget = ImClamp(framePixelWidthTarget, 0.1f, 50.f);

      framePixelWidth = ImLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

      frameCount = sequence->GetFrameMax() - sequence->GetFrameMin();
      if (visibleFrameCount >= frameCount && firstFrame)
         *firstFrame = sequence->GetFrameMin();


      // --
      if (expanded && !*expanded)
      {
         ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)TrackHeight));
         draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + TrackHeight), 0xFF3D3837, 0);
         char tmps[512];
         ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d Frames / %d entries", frameCount, trackCount);
         draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
      }
      else
      {
         bool hasScrollBar(true);
         /*
         int framesPixelWidth = int(frameCount * framePixelWidth);
         if ((framesPixelWidth + legendWidth) >= canvas_size.x)
         {
             hasScrollBar = true;
         }
         */
         // test scroll area
         ImVec2 headerSize(canvas_size.x, (float)TrackHeight);
         ImVec2 scrollBarSize(canvas_size.x, 14.f);
         ImGui::InvisibleButton("topBar", headerSize);
         draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);

         ImVec2 childFramePos = ImGui::GetCursorScreenPos();
         childFramePos = Max(canvas_pos, childFramePos); // Make sure that not going outside our frame
         ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
         ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
         ImGui::BeginChild("Sequencer Frame", childFrameSize);
         sequence->focused = ImGui::IsWindowFocused();
         ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, float(controlHeight)));
         const ImVec2 contentMin = ImGui::GetItemRectMin();
         const ImVec2 contentMax = ImGui::GetItemRectMax();
         const ImRect contentRect(contentMin, contentMax);
         const float contentHeight = contentMax.y - contentMin.y;

         // full background
         draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, 0xFF242424, 0);
         draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

         // current frame top
         ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + TrackHeight));

         mouseFrame = (int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed;
         mouseFrame = Clamp(mouseFrame, sequence->GetFrameMin(), sequence->GetFrameMax());


         if (!MovingCurrentFrame && !MovingScrollBar && movingTrack == -1 && movingKey == -1 && sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
         {
            MovingCurrentFrame = true;
         }
         if (MovingCurrentFrame)
         {
            if (frameCount)
            {
               *currentFrame = mouseFrame;
            }
            if (!io.MouseDown[0])
               MovingCurrentFrame = false;
         }

         //header
         draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + TrackHeight), 0xFF3D3837, 0);

         // Popup for adding a track
         if (sequenceOptions & SEQUENCER_ADD)
         {
            if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - TrackHeight, canvas_pos.y + 2), true) && io.MouseReleased[0])
            {
               ImGui::OpenPopup("addTrack");
            }

            if (ImGui::BeginPopup("addTrack"))
            {
               for (int i = 0; i < sequence->GetTrackTypeCount(); i++)
               {
                  if (ImGui::Selectable(sequence->GetTrackTypeName(i)))
                  {
                     int newTrack = sequence->AddTrack(i);
                     *selectedTrack = newTrack;
                     *selectedKey = -1;
                  }
               }

               ImGui::EndPopup();
               popupOpened = true;
            }
         }

         //header frame number and lines
         int modFrameCount = 10;
         int frameStep = 1;
         while ((modFrameCount * framePixelWidth) < 150)
         {
            modFrameCount *= 2;
            frameStep *= 2;
         };
         int halfModFrameCount = modFrameCount / 2;

         auto drawLine = [&](int i, int regionHeight) {
            bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence->GetFrameMax() || i == sequence->GetFrameMin());
            bool halfIndex = (i % halfModFrameCount) == 0;
            int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
            int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
            int tiretEnd = baseIndex ? regionHeight : TrackHeight;

            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
               draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

               draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)TrackHeight), ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
            }

            if (baseIndex && px > (canvas_pos.x + legendWidth))
            {
               char tmps[512];
               ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
               draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
            }

         };

         auto drawLineContent = [&](int i, int /*regionHeight*/) {
            int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
            int tiretStart = int(contentMin.y);
            int tiretEnd = int(contentMax.y);

            if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
            {
               //draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

               draw_list->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)), 0x30606060, 1);
            }
         };
         for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
         {
            drawLine(i, TrackHeight);
         }
         drawLine(sequence->GetFrameMin(), TrackHeight);
         drawLine(sequence->GetFrameMax(), TrackHeight);


         // Make is so that we don't scroll and overwrite the track header 
         draw_list->PushClipRect(canvas_pos + ImVec2(0.0f, TrackHeight), canvas_pos + canvas_size);

         // draw item names in the legend rect on the left
         size_t customHeight = 0;
         for (int i = 0; i < trackCount; i++)
         {
            ImVec2 startButtonPos(contentMin.x + 3, contentMin.y + i * TrackHeight + 2 + customHeight);

            next_button_data startButtonData = GetNextButtonData(startButtonPos.x, startButtonPos.y, 0, TrackHeight, false);
            bool overVisible = SequencerActiveButton(draw_list, ImVec2(startButtonData.ButtonX, startButtonData.ButtonY), sequence->IsTrackActive(i));
            if (overVisible && io.MouseReleased[0])
            {
               sequence->SetTrackActive(i, !sequence->IsTrackActive(i));
            }
            startButtonData = GetNextButtonData(startButtonPos.x, startButtonPos.y, startButtonData.NextButtonIdx, TrackHeight, false);
            ImVec2 startTextPos(startButtonData.ButtonX, startButtonData.ButtonY);
            draw_list->AddText(startTextPos, 0xFFFFFFFF, sequence->GetTrackLabel(i));

            if (sequenceOptions & SEQUENCER_DEL)
            {

               float baseX = contentMin.x + legendWidth - TrackHeight + 2 - 10;
               float baseY = startTextPos.y + 2;
               next_button_data nextButtonData = GetNextButtonData(baseX, baseY, 0, TrackHeight, true);
               bool overDel = SequencerAddDelButton(draw_list, ImVec2(nextButtonData.ButtonX, nextButtonData.ButtonY), false);
               if (overDel && io.MouseReleased[0])
                  delTrackIdx = i;

               nextButtonData = GetNextButtonData(baseX, baseY, nextButtonData.NextButtonIdx, TrackHeight, true);
               bool overAddFrame = SequencerAddDelButton(draw_list, ImVec2(nextButtonData.ButtonX, nextButtonData.ButtonY), true);
               if (overAddFrame && io.MouseReleased[0])
                  trackToAddKeyIdx = i;

               nextButtonData = GetNextButtonData(baseX, baseY, nextButtonData.NextButtonIdx, TrackHeight, true);
               bool overMoveDown = SequencerMoveDownButton(draw_list, ImVec2(nextButtonData.ButtonX, nextButtonData.ButtonY));
               if (overMoveDown && io.MouseReleased[0])
                  moveDownTrackIdx = i;

               nextButtonData = GetNextButtonData(baseX, baseY, nextButtonData.NextButtonIdx, TrackHeight, true);
               bool overMoveUp = SequencerMoveUpButton(draw_list, ImVec2(nextButtonData.ButtonX, nextButtonData.ButtonY));
               if (overMoveUp && io.MouseReleased[0])
                  moveUpTrackIdx = i;


               nextButtonData = GetNextButtonData(baseX, baseY, nextButtonData.NextButtonIdx, TrackHeight, true);
               bool overDup = SequencerCopyButton(draw_list, ImVec2(nextButtonData.ButtonX, nextButtonData.ButtonY), true);
               if (overDup && io.MouseReleased[0])
                  dupTrackIdx = i;
            }
            customHeight += sequence->GetCustomTrackHeight(i);
         }


         // selection
         bool isTrackSelected = selectedTrack && (*selectedTrack >= 0);

         // clipping rect so items bars are not visible in the legend on the left when scrolled
         //

         // Draw track backgrounds
         customHeight = 0;
         {
            for (int i = 0; i < trackCount; i++)
            {
               unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

               size_t localCustomHeight = sequence->GetCustomTrackHeight(i);
               ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + TrackHeight * i + 1 + customHeight);
               ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + TrackHeight - 1 + localCustomHeight);
               if (!popupOpened && cy >= pos.y && cy < pos.y + (TrackHeight + localCustomHeight) && movingTrack == -1 && movingKey == -1 && cx>contentMin.x && cx < contentMin.x + canvas_size.x)
               {
                  col += 0x80201008;
                  pos.x -= legendWidth;
               }
               draw_list->AddRectFilled(pos, sz, col, 0);
               customHeight += localCustomHeight;
            }


            // vertical frame lines in content area
            // NOTE(KCC): Vertical lines for the 
            for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
            {
               drawLineContent(i, int(contentHeight));
            }
            drawLineContent(sequence->GetFrameMin(), int(contentHeight));
            drawLineContent(sequence->GetFrameMax(), int(contentHeight));

            // Draw track selection background. Highlights the background of the selected track
            if (isTrackSelected)
            {
               customHeight = 0;
               for (int i = 0; i < *selectedTrack; i++)
               {
                  customHeight += sequence->GetCustomTrackHeight(i);;
               }

               draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + TrackHeight * *selectedTrack + customHeight), ImVec2(contentMin.x + canvas_size.x, contentMin.y + TrackHeight * (*selectedTrack + 1) + customHeight), TrackSelectionBackgroundColor, 1.f);
            }
         }


         // slots
         customHeight = 0;
         for (int trackIdx = 0; trackIdx < trackCount; trackIdx++)
         {
            size_t localCustomHeight = sequence->GetCustomTrackHeight(trackIdx);

            int keyCount = sequence->GetKeyCount(trackIdx);
            for (int keyIdx = 0; keyIdx < keyCount; keyIdx++)
            {
               // int* keyStart = nullptr;
               // int* keyEnd = nullptr;
               // unsigned int keyColor = 0;
               // sequencer_key_type::type keyType = sequencer_key_type::key;
               sequencer_key_data keyData = sequence->GetKeyData(trackIdx, keyIdx);
               if (*selectedTrack == trackIdx && *selectedKey == keyIdx)
               {
                  keyData.Color = keyData.Color | 0x0000DDDD;

               }
               // Calculate key rect
               ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + TrackHeight * trackIdx + 1 + customHeight);
               ImVec2 slotP1(pos.x + keyData.Start * framePixelWidth, pos.y + 2);
               ImVec2 slotP2(pos.x + keyData.End * framePixelWidth + framePixelWidth, pos.y + TrackHeight - 2);
               // To the end of the custom height
               ImVec2 slotP3(pos.x + keyData.End * framePixelWidth + framePixelWidth, pos.y + TrackHeight - 2 + localCustomHeight);
               unsigned int slotColor = keyData.Color | 0xFF000000;
               unsigned int slotColorHalf = (keyData.Color & 0xFFFFFF) | 0x40000000;

               // Draw the frame rects
               if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
               {
                  draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
                  draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
               }

               if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
               {
                  sequence->DoubleClickKey(trackIdx, keyIdx);
               }

               ImRect rects[3] = 
               { 
                  ImRect(slotP1, ImVec2(slotP1.x + framePixelWidth / 2, slotP2.y)),
                  ImRect(ImVec2(slotP2.x - framePixelWidth / 2, slotP1.y), slotP2),
                  ImRect(slotP1, slotP2),
               };

               const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slotColor + (isTrackSelected ? 0 : 0x202020) };
               if (movingTrack == -1  && movingKey == -1 && (sequenceOptions & SEQUENCER_EDIT_STARTEND))// TODOFOCUS && backgroundRect.Contains(io.MousePos))
               {
                  switch (keyData.KeyType)
                  {
                  case sequencer_key_type::key:
                     {
                        static_assert(2 < ArraySize(rects), "Need to update this.");
                        ImRect& rc = rects[2];
                        if (rc.Contains(io.MousePos))
                        {
                           draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[2], 2);
                           if (ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos) && 
                              ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame)
                           {
                              movingTrack = trackIdx;
                              movingKey = keyIdx;
                              movingPos = cx;
                              movingPart = 3;
                              sequence->BeginEdit(movingTrack, movingKey);
                           }
                        }
                     }
                     break;
                  case sequencer_key_type::range:
                     for (int j = 2; j >= 0; j--)
                     {
                        ImRect& rc = rects[j];
                        if (!rc.Contains(io.MousePos))
                           continue;
                        draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
                     }

                     for (int j = 0; j < ArraySize(rects); j++)
                     {
                        ImRect& rc = rects[j];
                        if (!rc.Contains(io.MousePos))
                           continue;
                        if (!ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
                           continue;
                        if (ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame)
                        {
                           movingTrack = trackIdx;
                           movingKey = keyIdx;
                           movingPos = cx;
                           movingPart = j + 1;
                           sequence->BeginEdit(movingTrack, movingKey);
                           break;
                        }
                     }
                     break;
                  }
               }
            }
            ImVec2 startPos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + TrackHeight * trackIdx + 1 + customHeight);
            ImVec2 endPos = ImVec2(contentMax.x, contentMin.y + TrackHeight * (trackIdx + 1) + 1 + customHeight);
            ImRect layerRect = ImRect(startPos, endPos);
            if (layerRect.Contains(io.MousePos))
            {
               if (io.MouseDoubleClicked[0])
               {
                  sequence->DoubleClickTrack(trackIdx);
               }
               if (ImGui::IsMouseClicked(0))
               {
                  if (*selectedTrack != trackIdx)
                  {
                     *selectedTrack = trackIdx;
                     *selectedKey = -1;
                  }
               }
            }

            // To the end of the custom height
            // moving
            if (movingTrack >= 0 && movingKey >= 0)
            {
               ImGui::CaptureMouseFromApp();
               int diffFrame = int((cx - movingPos) / framePixelWidth);
               if (std::abs(diffFrame) > 0)
               {
                  sequencer_key_data keyData = sequence->GetKeyData(movingTrack, movingKey);

                  if (selectedTrack)
                  {
                     *selectedTrack = movingTrack;
                  }
                  if (selectedKey)
                  {
                     *selectedKey = movingKey;
                  }

                  int l = keyData.Start;
                  int r = keyData.End;
                  if (movingPart & 1)
                     l += diffFrame;
                  if (movingPart & 2)
                     r += diffFrame;
                  if (l < 0)
                  {
                     if (movingPart & 2)
                        r -= l;
                     l = 0;
                  }
                  if (movingPart & 1 && l > r)
                     l = r;
                  if (movingPart & 2 && r < l)
                     r = l;

                  if (keyData.KeyType == sequencer_key_type::key)
                  {
                     r = l;
                  }
                  movingKey = sequence->MoveKey(movingTrack, movingKey, l, r);
                  movingPos += int(diffFrame * framePixelWidth);
               }
               if (!io.MouseDown[0])
               {
                  // single select
                  if (!diffFrame && movingPart && selectedTrack && selectedKey)
                  {
                     *selectedTrack = movingTrack;
                     *selectedKey = movingKey;
                     ret = true;
                  }

                  sequence->EndEdit(movingTrack, movingKey);
                  movingTrack = -1;
                  movingKey = -1;
               }
            }

            // custom draw
            if (localCustomHeight > 0)
            {
               ImVec2 rp(canvas_pos.x, contentMin.y + TrackHeight * trackIdx + 1 + customHeight);
               ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(TrackHeight)),
                  rp + ImVec2(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(localCustomHeight + TrackHeight)));
               ImRect clippingRect(rp + ImVec2(float(legendWidth), float(TrackHeight)), rp + ImVec2(canvas_size.x, float(localCustomHeight + TrackHeight)));

               ImRect legendRect(rp + ImVec2(0.f, float(TrackHeight)), rp + ImVec2(float(legendWidth), float(localCustomHeight)));
               ImRect legendClippingRect(canvas_pos + ImVec2(0.f, float(TrackHeight)), canvas_pos + ImVec2(float(legendWidth), float(localCustomHeight + TrackHeight)));
               customDraws.push_back({ trackIdx, customRect, legendRect, clippingRect, legendClippingRect });
            }
            else
            {
               ImVec2 rp(canvas_pos.x, contentMin.y + TrackHeight * trackIdx + customHeight);
               ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(0.f)),
                  rp + ImVec2(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(TrackHeight)));
               ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(TrackHeight)));

               compactCustomDraws.push_back({ trackIdx, customRect, ImRect(), clippingRect, ImRect() });
            }
            customHeight += localCustomHeight;
         }

         draw_list->PopClipRect();
         draw_list->PopClipRect();

         for (auto& customDraw : customDraws)
            sequence->CustomDraw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect);
         for (auto& customDraw : compactCustomDraws)
            sequence->CustomDrawCompact(customDraw.index, draw_list, customDraw.customRect, customDraw.clippingRect);

         // copy paste
         if (sequenceOptions & SEQUENCER_COPYPASTE)
         {
            ImRect rectCopy(ImVec2(contentMin.x + 100, canvas_pos.y + 2)
               , ImVec2(contentMin.x + 100 + 30, canvas_pos.y + TrackHeight - 2));
            bool inRectCopy = rectCopy.Contains(io.MousePos);
            unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectCopy.Min, copyColor, "Copy");

            ImRect rectPaste(ImVec2(contentMin.x + 140, canvas_pos.y + 2)
               , ImVec2(contentMin.x + 140 + 30, canvas_pos.y + TrackHeight - 2));
            bool inRectPaste = rectPaste.Contains(io.MousePos);
            unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

            if (inRectCopy && io.MouseReleased[0])
            {
               sequence->Copy();
            }
            if (inRectPaste && io.MouseReleased[0])
            {
               sequence->Paste();
            }
         }
         //

         // Red Current Frame Bar (Shows the current frame)
         if (currentFrame && firstFrame && *currentFrame >= *firstFrame && *currentFrame <= sequence->GetFrameMax())
         {
            static const float cursorWidth = 8.f;
            float cursorOffset = contentMin.x + legendWidth + (*currentFrame - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
            draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y), 0xA02A2AFF, cursorWidth);
            char tmps[512];
            ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", *currentFrame);
            draw_list->AddText(ImVec2(cursorOffset + 10, canvas_pos.y + 2), 0xFF2A2AFF, tmps);
         }

         ImGui::EndChild();

         ImGui::PopStyleColor();
         if (hasScrollBar)
         {
            ImGui::InvisibleButton("scrollBar", scrollBarSize);
            ImVec2 scrollBarMin = ImGui::GetItemRectMin();
            ImVec2 scrollBarMax = ImGui::GetItemRectMax();

            // ratio = number of frames visible in control / number to total frames

            float startFrameOffset = ((float)(firstFrameUsed - sequence->GetFrameMin()) / (float)frameCount) * (canvas_size.x - legendWidth);
            ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
            ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

            ImRect scrollBarRect(scrollBarA, scrollBarB);
            bool inScrollBar = scrollBarRect.Contains(io.MousePos);

            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


            ImVec2 scrollBarC(scrollBarMin.x + legendWidth + startFrameOffset, scrollBarMin.y);
            ImVec2 scrollBarD(scrollBarMin.x + legendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
            draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

            ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
            ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

            bool onLeft = barHandleLeft.Contains(io.MousePos);
            bool onRight = barHandleRight.Contains(io.MousePos);

            static bool sizingRBar = false;
            static bool sizingLBar = false;

            draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666, 6);
            draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666, 6);

            ImRect scrollBarThumb(scrollBarC, scrollBarD);
            static const float MinBarWidth = 44.f;
            if (sizingRBar)
            {
               if (!io.MouseDown[0])
               {
                  sizingRBar = false;
               }
               else
               {
                  float barNewWidth = ImMax(barWidthInPixels + io.MouseDelta.x, MinBarWidth);
                  float barRatio = barNewWidth / barWidthInPixels;
                  framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                  int newVisibleFrameCount = int((canvas_size.x - legendWidth) / framePixelWidthTarget);
                  int lastFrame = *firstFrame + newVisibleFrameCount;
                  if (lastFrame > sequence->GetFrameMax())
                  {
                     framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(sequence->GetFrameMax() - *firstFrame);
                  }
               }
            }
            else if (sizingLBar)
            {
               if (!io.MouseDown[0])
               {
                  sizingLBar = false;
               }
               else
               {
                  if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
                  {
                     float barNewWidth = ImMax(barWidthInPixels - io.MouseDelta.x, MinBarWidth);
                     float barRatio = barNewWidth / barWidthInPixels;
                     float previousFramePixelWidthTarget = framePixelWidthTarget;
                     framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                     int newVisibleFrameCount = int(visibleFrameCount / barRatio);
                     int newFirstFrame = *firstFrame + newVisibleFrameCount - visibleFrameCount;
                     newFirstFrame = ImClamp(newFirstFrame, sequence->GetFrameMin(), ImMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
                     if (newFirstFrame == *firstFrame)
                     {
                        framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
                     }
                     else
                     {
                        *firstFrame = newFirstFrame;
                     }
                  }
               }
            }
            else
            {
               if (MovingScrollBar)
               {
                  if (!io.MouseDown[0])
                  {
                     MovingScrollBar = false;
                  }
                  else
                  {
                     float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
                     *firstFrame = int((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - panningViewFrame;
                     *firstFrame = ImClamp(*firstFrame, sequence->GetFrameMin(), ImMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
                  }
               }
               else
               {
                  if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && firstFrame && !MovingCurrentFrame && movingTrack == -1 && movingKey == -1)
                  {
                     MovingScrollBar = true;
                     panningViewSource = io.MousePos;
                     panningViewFrame = -*firstFrame;
                  }
                  if (!sizingRBar && onRight && ImGui::IsMouseClicked(0))
                     sizingRBar = true;
                  if (!sizingLBar && onLeft && ImGui::IsMouseClicked(0))
                     sizingLBar = true;

               }
            }
         }
      }

      ImGui::EndGroup();

      if (regionRect.Contains(io.MousePos))
      {
         bool overCustomDraw = false;
         for (auto& custom : customDraws)
         {
            if (custom.customRect.Contains(io.MousePos))
            {
               overCustomDraw = true;
            }
         }
         if (overCustomDraw)
         {
         }
         else
         {
#if 0
            frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
            //frameOverCursor = max(min(*firstFrame - visibleFrameCount / 2, frameCount - visibleFrameCount), 0);

            /**firstFrame -= frameOverCursor;
            *firstFrame *= framePixelWidthTarget / framePixelWidth;
            *firstFrame += frameOverCursor;*/
            if (io.MouseWheel < -FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 1.1f);
               framePixelWidthTarget *= 0.9f;
               *firstFrame += frameOverCursor;
            }

            if (io.MouseWheel > FLT_EPSILON)
            {
               *firstFrame -= frameOverCursor;
               *firstFrame = int(*firstFrame * 0.9f);
               framePixelWidthTarget *= 1.1f;
               *firstFrame += frameOverCursor;
            }
#endif
         }
      }

      if (expanded)
      {
         bool overExpanded = SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded);
         if (overExpanded && io.MouseReleased[0])
            *expanded = !*expanded;
      }

      if (delTrackIdx != -1)
      {
         sequence->DeleteTrack(delTrackIdx);
         if (selectedTrack && (*selectedTrack == delTrackIdx || *selectedTrack >= sequence->GetTrackCount()))
            *selectedTrack = -1;
      }

      if (dupTrackIdx != -1)
      {
         sequence->DuplicateTrack(dupTrackIdx);
      }

      if (trackToAddKeyIdx != -1)
      {
         *selectedTrack = trackToAddKeyIdx;
         *selectedKey = sequence->AddKey(trackToAddKeyIdx, 0, 0);
      }

      if (moveUpTrackIdx != -1)
      {
         *selectedTrack = sequence->MoveTrackUp(moveUpTrackIdx);
         *selectedKey = -1;
      }

      if (moveDownTrackIdx != -1)
      {
         *selectedTrack = sequence->MoveTrackDown(moveDownTrackIdx);
         *selectedKey = -1;
      }

      if (ImGui::IsKeyReleased(VK_DELETE) && *selectedTrack >= 0 && *selectedKey >= 0)
      {
         sequence->DeleteKey(*selectedTrack, *selectedKey);
         *selectedTrack = -1;
         *selectedKey = -1;

         movingTrack = -1;
         movingKey = -1;
      }

      if (ImGui::IsKeyReleased(VK_SPACE) && *selectedTrack >= 0 && mouseFrame >= 0)
      {
         *selectedKey = sequence->AddKey(*selectedTrack, mouseFrame, mouseFrame);
         movingTrack = -1;
         movingKey = -1;
      }
      if (ImGui::IsKeyReleased(VK_ESCAPE))
      {
         *selectedTrack = -1;
         *selectedKey = -1;
         movingTrack = -1;
         movingKey = -1;
      }

      if (ImGui::IsKeyReleased(VK_MENU) && *selectedTrack >= 0 && *selectedKey >= 0)
      {
         *selectedKey = sequence->DuplicateKey(*selectedTrack, *selectedKey);

         movingTrack = -1;
         movingKey = -1;
      }
      return ret;
   }
}
