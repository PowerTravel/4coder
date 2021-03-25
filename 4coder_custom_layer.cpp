/*
4coder_default_bidings.cpp - Supplies the default bindings used for default 4coder behavior.
*/

/* TODO(Jakob):  
*  - Multi-row editing
*  - Vim-style BookMarks
*  - Progressbar showing how far down you are in a file
*  - Close build panel on return code 0 (sucess)
*  - More syntax highlightnig, macros, if-else-for-case-class-struct, Functions,etc 
*  - Make top-file-bar grayed out when window is not in focus
 */

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.

#include "4coder_custom_hooks.cpp"

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

void
custom_layer_init(Application_Links *app){
  Thread_Context *tctx = get_thread_context(app);
  
  // NOTE(allen): setup for default framework
  default_framework_init(app);
  
  // NOTE(allen): default hooks and command maps
  set_all_default_hooks(app);

  set_custom_hook(app, HookID_RenderCaller, custom_render_caller);

  mapping_init(tctx, &framework_mapping);
  String_ID global_map_id = vars_save_string_lit("keys_global");
  String_ID file_map_id = vars_save_string_lit("keys_file");
  String_ID code_map_id = vars_save_string_lit("keys_code");
  
  setup_default_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
  
  setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
}

#endif //FCODER_DEFAULT_BINDINGS
