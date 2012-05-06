<?php

/**
 * @file
 * Please supply a file description.
 */

/**
 * Implements hook_preprocess_html().
 */
function root_preprocess_html(&$variables) {
  // Clean up the attributes for the html element.
  $variables['attributes_array'] = array(
    'lang' => $variables['language']->language,
    'dir' => $variables['language']->dir,
  );

  // Add some styles in case the Root theme is the active theme.
  if ($GLOBALS['theme'] == 'root') {
    _root_preprocess_html($variables);
  }
}

/**
 * Helper function for loading theme assets in case 'Root' is the active theme.
 *
 * @see root_preprocess_html()
 */
function _root_preprocess_html(&$variables) {
  // @todo
}
