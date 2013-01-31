<?php


/**
 * @file
 * Contains the block attachment display plugin.
 */

/**
 * The plugin that handles an attachment display, but renders it in a block.
 *
 * Attachment displays are secondary displays that are 'attached' to a primary
 * display. Effectively they are a simple way to get multiple views within
 * the same view. They can share some information.
 *
 * @see views_plugin_display_attachment.
 * @ingroup views_display_plugins
 */
class views_attachment_block_plugin_display_block_attachment extends views_plugin_display_attachment {

  function option_definition() {
    $options = parent::option_definition();

    unset($options['attachment_position']);

    $options['block_description'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  /**
   * Provide the summary for page options in the views UI.
   *
   * This output is returned as an array.
   */
  function options_summary(&$categories, &$options) {
    // It is very important to call the parent function here:
    parent::options_summary($categories, $options);

    unset($options['attachment_position']);

    $categories['block'] = array(
      'title' => t('Block settings'),
    );

    $block_description = strip_tags($this->get_option('block_description'));
    if (empty($block_description)) {
      $block_description = t('None');
    }

    if (strlen($block_description) > 16) {
      $block_description = substr($block_description, 0, 16) . '...';
    }

    $options['block_description'] = array(
      'category' => 'block',
      'title' => t('Admin'),
      'value' => $block_description,
    );
  }

  /**
   * Provide the default form for setting options.
   */
  function options_form(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_form($form, $form_state);

    switch ($form_state['section']) {
      case 'block_description':
        $form['#title'] .= t('Block admin description');
        $form['block_description'] = array(
          '#type' => 'textfield',
          '#description' => t('This will appear as the name of this block in administer >> site building >> blocks.'),
          '#default_value' => $this->get_option('block_description'),
        );
        break;
    }
  }

  /**
   * Perform any necessary changes to the form values prior to storage.
   * There is no need for this function to actually store the data.
   */
  function options_submit(&$form, &$form_state) {
    // It is very important to call the parent function here:
    parent::options_submit($form, $form_state);
    switch ($form_state['section']) {
      case 'block_description':
        $this->set_option('block_description', $form_state['values']['block_description']);
        break;
    }
  }

  /**
   * The default block handler doesn't support configurable items,
   * but extended block handlers might be able to do interesting
   * stuff with it.
   */
  function execute_hook_block_list($delta = 0, $edit = array()) {
    $delta = $this->view->name . '-' . $this->display->id;
    $desc = $this->get_option('block_description');

    if (empty($desc)) {
      $desc = t('@view: @display', array('@view' => $this->view->name, '@display' => $this->display->display_title));
    }
    return array(
      $delta => array(
        'info' => $desc,
        'cache' => BLOCK_NO_CACHE,
      )
    );
  }

  /**
   * The display block handler returns the structure necessary for a block.
   */
  function execute() {
    if (!empty($this->view->is_attachment)) {
      // The view is being executed on another view.
      return $this->view->render($this->display->id);
     //$view = views_get_page_view();

    }
    else {
      $view = views_get_page_view();
      if (is_object($view)) {
        $content = $view->build_info['blocks_attached'][$this->display->id];
        // This is a hook_block implementation calling:
        // Prior to this being called, the $view should already be set to this
        // display, and arguments should be set on the view.
        $info['content'] = $content;
        $info['subject'] = filter_xss_admin($this->view->get_title());
        if (!empty($content) || $this->get_option('empty') || !empty($this->view->style_plugin->definition['even empty'])) {
          return $info;
        }
      }
    }

  }

  /**
   * Attach to another view.
   */
  function attach_to($display_id) {
    $displays = $this->get_option('displays');

    if (empty($displays[$display_id])) {
      return;
    }

    if (!$this->access()) {
      return;
    }

    // Get a fresh view because our current one has a lot of stuff on it because it's
    // already been executed.
    $view = $this->view->clone_view();
    $view->original_args = $view->args;

    $args = $this->get_option('inherit_arguments') ? $this->view->args : array();
    $view->set_arguments($args);
    $view->set_display($this->display->id);
    if ($this->get_option('inherit_pager')) {
      $view->display_handler->use_pager = $this->view->display[$display_id]->handler->use_pager();
      $view->display_handler->set_option('pager_element', $this->view->display[$display_id]->handler->get_option('pager_element'));
    }

    // because of this, it is very very important that displays that can accept
    // attachments not also be attachments, or this could explode messily.
    $attachment = $view->execute_display($this->display->id, $args);

    // Store the rendered view in the build info for later retreval:
    $this->view->build_info['blocks_attached'][$this->display->id] = $attachment;

    $view->destroy();
  }

  /**
   * Check to see which display to use when creating links within
   * a view using this display.
   */
  function get_link_display() {
    $display_id = $this->get_option('link_display');

    if (empty($display_id) || empty($this->view->display[$display_id])) {
      // Start by looking at the views we're attached to;
      if (is_array($this->view->old_view)) {
        foreach (array_reverse($this->view->old_view) as $old_view) {
          if (!empty($old_view->current_display)) {
            return $old_view->current_display;
          }
        }
      }

      // Fall back to the first display handler with a path:
      foreach ($this->view->display as $display_id => $display) {
        if (!empty($display->handler) && $display->handler->has_path()) {
          return $display_id;
        }
      }
    }
    else {
      return $display_id;
    }
    // fall-through returns NULL
  }

}