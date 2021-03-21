
typedef struct Highlight_Pair {
  String_Const_u8 needle;
  ARGB_Color color;
} Highlight_Pair;

// NOTE(Jakob): Helper function from https://4coder.handmade.network/wiki/7319-customization_layer_-_getting_started__4coder_4.1_
/* NOTE: based on draw_comment_highlights. */
function void draw_keyword_highlights( Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Token_Array *array, Highlight_Pair *pairs, i32 pair_count ) {
  Scratch_Block scratch( app );
  Range_i64 visible_range = text_layout_get_visible_range( app, text_layout_id );
  i64 first_index = token_index_from_pos( array, visible_range.first );
  Token_Iterator_Array it = token_iterator_index( buffer, array, first_index );
  
  for ( ; ; ) {
    
    Temp_Memory_Block temp( scratch );
    Token *token = token_it_read( &it );
    if ( token->pos >= visible_range.one_past_last ){
      break;
    }
    
    String_Const_u8 tail = { 0 };
    
    if ( token_it_check_and_get_lexeme( app, scratch, &it, TokenBaseKind_Identifier, &tail ) ){
      
      Highlight_Pair *pair = pairs;
      
      for ( i32 i = 0; i < pair_count; i += 1, pair += 1 ) {
        
        if ( string_match( tail, pair->needle ) ) {
          Range_i64 range = Ii64_size( token->pos, token->size );
          paint_text_color( app, text_layout_id, range, pair->color );
          break;
        }
      }
    }
    
    if ( !token_it_inc_non_whitespace( &it ) ){
      break;
    }
  }
}
// NOTE(Jakob): Helper function from https://4coder.handmade.network/wiki/7319-customization_layer_-_getting_started__4coder_4.1_
function void draw_string_highlights( Application_Links *app, Buffer_ID buffer, Text_Layout_ID text_layout_id, Highlight_Pair *pairs, i32 pair_count ) {
  
  Range_i64 visible_range = text_layout_get_visible_range( app, text_layout_id );
  
  Highlight_Pair* pair = pairs;
  
  for ( i32 i = 0; i < pair_count; i += 1, pair += 1 ) {
    
    if ( pair->needle.size <= 0 ) {
      continue;
    }
    
    i64 position = visible_range.min;
    seek_string_insensitive_forward( app, buffer, position - 1, visible_range.max, pair->needle, &position );
    
    while ( position < visible_range.max ) {
      
      Range_i64 range = Ii64_size( position, pair->needle.size );
      paint_text_color( app, text_layout_id, range, pair->color );
      seek_string_insensitive_forward( app, buffer, position, visible_range.max, pair->needle, &position );
    }
  }
}

function void
custom_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                     Buffer_ID buffer, Text_Layout_ID text_layout_id,
                     Rect_f32 rect){
  ProfileScope(app, "render buffer");
  
  View_ID active_view = get_active_view(app, Access_Always);
  b32 is_active_view = (active_view == view_id);
  Rect_f32 prev_clip = draw_set_clip(app, rect);
  
  Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
  
  // NOTE(allen): Cursor shape
  Face_Metrics metrics = get_face_metrics(app, face_id);
  f32 cursor_roundness = metrics.normal_advance*global_config.cursor_roundness;
  f32 mark_thickness = (f32)global_config.mark_thickness;
  
  // NOTE(allen): Token colorizing
  Token_Array token_array = get_token_array_from_buffer(app, buffer);
  if (token_array.tokens != 0){
    draw_cpp_token_colors(app, text_layout_id, &token_array);
    
    // NOTE(allen): Scan for TODOs and NOTEs
    if (global_config.use_comment_keyword){
      Comment_Highlight_Pair pairs[] = {
        {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
        {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
      };
      draw_comment_highlights(app, buffer, text_layout_id,
                              &token_array, pairs, ArrayCount(pairs));
    }
    
    Highlight_Pair string_pairs[ ] = {
      string_u8_litexpr( "some string" ), finalize_color( defcolor_keyword, 0 ), /* Use theme color "defcolor_keyword" first color. */
      string_u8_litexpr( "other string" ), 0xffa46391, /* Hardcoded colors work too. */
    };
    
    draw_string_highlights( app, buffer, text_layout_id, string_pairs, ArrayCount( string_pairs ) );
    
    // NOTE(Jakob): Default tokens
    Highlight_Pair token_pairs[] = {
#include "keyWords.h"
    };
    
    draw_keyword_highlights(app, buffer, text_layout_id, &token_array, token_pairs, ArrayCount(token_pairs));
  }
  else{
    paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
  }
  
  i64 cursor_pos = view_correct_cursor(app, view_id);
  view_correct_mark(app, view_id);
  
  // NOTE(allen): Scope highlight
  if (global_config.use_scope_highlight){
    Color_Array colors = finalize_color_array(defcolor_back_cycle);
    draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
  }
  
  if (global_config.use_error_highlight || global_config.use_jump_highlight){
    // NOTE(allen): Error highlight
    String_Const_u8 name = string_u8_litexpr("*compilation*");
    Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
    if (global_config.use_error_highlight){
      draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                           fcolor_id(defcolor_highlight_junk));
    }
    
    // NOTE(allen): Search highlight
    if (global_config.use_jump_highlight){
      Buffer_ID jump_buffer = get_locked_jump_buffer(app);
      if (jump_buffer != compilation_buffer){
        draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                             fcolor_id(defcolor_highlight_white));
      }
    }
  }
  
  // NOTE(allen): Color parens
  if (global_config.use_paren_helper){
    Color_Array colors = finalize_color_array(defcolor_text_cycle);
    draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
  }
  
  // NOTE(allen): Line highlight
  if (global_config.highlight_line_at_cursor && is_active_view){
    i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
    draw_line_highlight(app, text_layout_id, line_number,
                        fcolor_id(defcolor_highlight_cursor_line));
  }
  
  // NOTE(allen): Whitespace highlight
  b64 show_whitespace = false;
  view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
  if (show_whitespace){
    if (token_array.tokens == 0){
      draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
    }
    else{
      draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
    }
  }
  
  // NOTE(allen): Cursor
  switch (fcoder_mode){
    case FCoderMode_Original:
    {
      draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
    }break;
    case FCoderMode_NotepadLike:
    {
      draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
    }break;
  }
  
  // NOTE(allen): Fade ranges
  paint_fade_ranges(app, text_layout_id, buffer);
  
  // NOTE(allen): put the actual text on the actual screen
  draw_text_layout_default(app, text_layout_id);
  
  draw_set_clip(app, prev_clip);
}

function void
custom_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
  
  ProfileScope(app, "default render caller");
  
  View_ID active_view = get_active_view(app, Access_Always);
  b32 is_active_view = (active_view == view_id);
  
  Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
  Rect_f32 prev_clip = draw_set_clip(app, region);
  
  Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
  Face_ID face_id = get_face_id(app, buffer);
  Face_Metrics face_metrics = get_face_metrics(app, face_id);
  f32 line_height = face_metrics.line_height;
  f32 digit_advance = face_metrics.decimal_digit_advance;
  
  // NOTE(allen): file bar
  b64 showing_file_bar = false;
  if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
    Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
    draw_file_bar(app, view_id, buffer, face_id, pair.min);
    region = pair.max;
  }
  
  Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
  
  Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
                                                frame_info.animation_dt, scroll);
  if (!block_match_struct(&scroll.position, &delta.point)){
    block_copy_struct(&scroll.position, &delta.point);
    view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
  }
  if (delta.still_animating){
    animate_in_n_milliseconds(app, 0);
  }
  
  // NOTE(allen): query bars
  region = default_draw_query_bars(app, region, view_id, face_id);
  
  // NOTE(allen): FPS hud
  if (show_fps_hud){
    Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
    draw_fps_hud(app, frame_info, face_id, pair.max);
    region = pair.min;
    animate_in_n_milliseconds(app, 1000);
  }
  
  // NOTE(allen): layout line numbers
  Rect_f32 line_number_rect = {};
  if (global_config.show_line_number_margins){
    Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
    line_number_rect = pair.min;
    region = pair.max;
  }
  
  // NOTE(allen): begin buffer render
  Buffer_Point buffer_point = scroll.position;
  Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
  
  // NOTE(allen): draw line numbers
  if (global_config.show_line_number_margins){
    draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
  }
  
  // NOTE(Jakob): Our custom render_buffer
  custom_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);
  
  text_layout_free(app, text_layout_id);
  draw_set_clip(app, prev_clip);
  
}

// BOTTOM

