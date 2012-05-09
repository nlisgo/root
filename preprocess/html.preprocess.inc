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

  // Classes for body element. Allows advanced theming based on context
  // (home page, node of certain type, etc.).
  if (!$variables['is_front']) {
    // Add unique class for each page.
    $path = drupal_get_path_alias($_GET['q']);
    // Add unique class for each section of the website.
    list($section, ) = explode('/', $path, 2);
    $arg = explode('/', $_GET['q']);

    if ($arg[0] == 'node' && isset($arg[1])) {
      if ($arg[1] == 'add') {
        $section = 'node-add';
      }
      elseif (isset($arg[2]) && is_numeric($arg[1]) && ($arg[2] == 'edit' || $arg[2] == 'delete')) {
        $section = 'node-' . $arg[2];
      }
    }

    $variables['attributes_array']['class'][] = drupal_html_class('section-' . $section);
  }

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
