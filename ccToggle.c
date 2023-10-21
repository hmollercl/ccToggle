#include "./uris.h"

#include "lv2/atom/atom.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { MIDI_IN = 0, MIDI_OUT = 1, CHANNEL = 2 };

typedef struct {
    // Features
    LV2_URID_Map*  map;
    LV2_Log_Logger logger;

    // Ports
    const LV2_Atom_Sequence* in_port;
    LV2_Atom_Sequence*       out_port;
    int* channel_ptr;

    // URIs
    FifthsURIs uris;

    int memCI[16][128]
}Fifths;

static void connect_port(LV2_Handle instance, uint32_t port, void* data){
    Fifths* self = (Fifths*)instance;
    
    switch (port){
    case MIDI_IN:
        self->in_port = (const LV2_Atom_Sequence*)data;
        break;
    case MIDI_OUT:
        self->out_port = (LV2_Atom_Sequence*)data;
        break;
    default:
        break;
    }
}

static LV2_Handle instantiate(const struct LV2_Descriptor* descriptor,
            double                    rate,
            const char*               path,
            const LV2_Feature* const* features){
    // Allocate and initialise instance structure.
    Fifths* self = (Fifths*)calloc(1, sizeof(Fifths));
    if (!self) {
        return NULL;
    }

    // Scan host features for URID map
    // clang-format off
    const char*  missing = lv2_features_query(
        features,
        LV2_LOG__log,  &self->logger.log, false,
        LV2_URID__map, &self->map,        true,
        NULL);
    // clang-format on

    lv2_log_logger_set_map(&self->logger, self->map);
    if (missing) {
        lv2_log_error(&self->logger, "Missing feature <%s>\n", missing);
        free(self);
        return NULL;
    }

    //map_fifths_uris(self->map, &self->uris);  // what?

    // TODO maybe this in activate (need to create function and call)
    // initialice the state of toggle normal off
    int c,k;  
    for (c=0; c < 16; ++c) for (k=0; k < 127; ++k){
		self->memCI[c][k] = 0;
	}

    return (LV2_Handle)self;
}

static void cleanup(LV2_Handle instance){
    free(instance);
}

static void
run(LV2_Handle instance, uint32_t sample_count)
{
  Fifths*     self = (Fifths*)instance;
  FifthsURIs* uris = &self->uris;

  // Struct for a 3 byte MIDI event, used for writing notes
  typedef struct {
    LV2_Atom_Event event;
    uint8_t        msg[3];
  } MIDINoteEvent;

  // Initially self->out_port contains a Chunk with size set to capacity

  // Get the capacity
  const uint32_t out_capacity = self->out_port->atom.size;

  // Write an empty Sequence header to the output
  lv2_atom_sequence_clear(self->out_port);
  self->out_port->atom.type = self->in_port->atom.type;

  // Read incoming events
  LV2_ATOM_SEQUENCE_FOREACH (self->in_port, ev) {
    if (ev->body.type == uris->midi_Event) {
      const uint8_t* const msg = (const uint8_t*)(ev + 1);
      switch (lv2_midi_message_type(msg)) {
        case LV2_MIDI_MSG_CONTROLLER:
          if(self->memCI[msg[*self->channel_ptr]][msg[2]]<64)
            lv2_atom_sequence_append_event(self->out_port, out_capacity, 127);
          else
            lv2_atom_sequence_append_event(self->out_port, out_capacity, 0);
          break;
      default:
        // Forward all other MIDI events directly
        lv2_atom_sequence_append_event(self->out_port, out_capacity, ev);
        break;
      }
    }
  }
}

static const void*
extension_data(const char* uri)
{
  return NULL;
}

static const LV2_Descriptor descriptor = {CCTOGGLE_URI,
                                          instantiate,
                                          connect_port,
                                          NULL, // activate,
                                          run,
                                          NULL, // deactivate,
                                          cleanup,
                                          extension_data};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index)
{
  return index == 0 ? &descriptor : NULL;
}

