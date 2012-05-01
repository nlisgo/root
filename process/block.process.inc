<?php

/**
 * @file
 * Please supply a file description.
 */

/**
 * Implements hook_process_block().
 */
function root_process_block(&$variables) {
  // Drupal 7 should use a $title variable instead of $block->subject.
  $variables['title'] = $variables['block']->subject;
}
