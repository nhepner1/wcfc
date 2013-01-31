<?php

/**
 * Implementation of hook_views_plugins().
 */
function views_attachment_block_views_plugins() {
  $plugins = array(
    'display' => array(
      'block_attachment' => array(
        'title' => t('Block attachment'),
        'help' => t('Attachments added to other displays and then rendered in a block.'),
        'handler' => 'views_attachment_block_plugin_display_block_attachment',
        'theme' => 'views_view',
        'use ajax' => TRUE,
        'help topic' => 'display-block-attachment',
        'parent' => 'attachment',
        'uses hook block' => TRUE,
      ),
    ),
  );

  return $plugins;
}
