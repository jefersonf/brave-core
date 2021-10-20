/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * you can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/modules/speech/speech_synthesis.h"

#define OnSetVoiceList OnSetVoiceList_ChromiumImpl
#include "../../../../../../../third_party/blink/renderer/modules/speech/speech_synthesis.cc"
#undef OnSetVoiceList

namespace blink {

void SpeechSynthesis::OnSetVoiceList(
    Vector<mojom::blink::SpeechSynthesisVoicePtr> mojom_voices) {
  voice_list_.clear();
  mojom::blink::SpeechSynthesisVoicePtr fake_voice = nullptr;
  std::vector<WTF::String> fake_names = {
      "Hubert", "Vernon", "Rudolph",   "Clayton",    "Irving",
      "Wilson", "Alva",   "Harley",    "Beauregard", "Cleveland",
      "Cecil",  "Reuben", "Sylvester", "Jasper"};
  for (auto& mojom_voice : mojom_voices) {
    if (!fake_voice && mojom_voice->is_default) {
      if (ExecutionContext* context = GetExecutionContext()) {
        fake_voice = mojom_voice.Clone();
        fake_voice->is_default = false;
        std::mt19937_64 prng = brave::BraveSessionCache::From(*context)
                                   .MakePseudoRandomGenerator();
        fake_voice->name = fake_names[(prng() % fake_names.size())];
      }
    }
    voice_list_.push_back(
        MakeGarbageCollected<SpeechSynthesisVoice>(std::move(mojom_voice)));
  }
  if (fake_voice) {
    voice_list_.push_back(
        MakeGarbageCollected<SpeechSynthesisVoice>(std::move(fake_voice)));
  }
  VoicesDidChange();
}

}  // namespace blink
