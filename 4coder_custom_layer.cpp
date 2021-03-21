/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

/* TODO(Jakob):  
     *  - Project-wide function jumping
*  - Multi-row editing
*  - Vim-style BookMarks
*  - Progressbar showing how far down you are in a file
 */

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.

#include "4coder_custom_hooks.cpp"

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif
some string
other string
void
custom_layer_init(Application_Links *app){
  Thread_Context *tctx = get_thread_context(app);
  
  // NOTE(allen): setup for default framework
  default_framework_init(app);
  
  // NOTE(allen): default hooks and command maps
  set_all_default_hooks(app);
  
#if 1
  set_custom_hook(app, HookID_RenderCaller, custom_render_caller);
#else
  set_custom_hook(app, HookID_RenderCaller, default_render_caller);
#endif
  
  mapping_init(tctx, &framework_mapping);
  setup_default_mapping(&framework_mapping, mapid_global, mapid_file, mapid_code);
  
  // NOTE(jakob): custom key-bindings to the global_map
  MappingScope();
  SelectMapping(&framework_mapping);
  
  SelectMap(mapid_global);
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

