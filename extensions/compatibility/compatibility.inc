<?php

/**
 * @file
 * Main extension file for the 'compatibility' extension.
 */

/**
 * Implements hook_extension_EXTENSION_registry_alter().
 */
function root_extension_compatibility_theme_registry_alter(&$registry) {
  $registry['html']['includes'][] = drupal_get_path('theme', 'root') . '/extensions/compatibility/compatibility.inc';
  $registry['html']['preprocess functions'][] = 'root_extension_compatibility_preprocess_html';
}

/**
 * Implements hook_extension_EXTENSION_preproces_html().
 */
function root_extension_compatibility_preprocess_html(&$variables) {
  // Add the 'viewport' meta tag.
  if (theme_get_setting('root_viewport')) {
    $meta = array(
      '#tag' => 'meta',
      '#attributes' => array('name' => 'viewport', 'content' => 'width=device-width'),
    );

    drupal_add_html_head($meta, 'root-viewport');
  }

  // Add the 'HandheldFriendly' meta tag.
  if (theme_get_setting('root_handheld_friendly')) {
    $meta = array(
      '#tag' => 'meta',
      '#attributes' => array('name' => 'HandheldFriendly', 'content' => 'true'),
    );

    drupal_add_html_head($meta, 'root-handheld-friendly');
  }

  // Add a the 'MobileOptimized' meta tag.
  if (theme_get_setting('root_mobile_optimized')) {
    $meta = array(
      '#tag' => 'meta',
      '#attributes' => array('name' => 'MobileOptimized', 'content' => 'width'),
    );

    drupal_add_html_head($meta, 'root-mobile-optimized');
  }

  // Add the 'cleartype' meta tag.
  if (theme_get_setting('root_cleartype')) {
    $meta = array(
      '#tag' => 'meta',
      '#attributes' => array('http-equiv' => 'cleartype', 'content' => 'on'),
    );

    drupal_add_html_head($meta, 'root-cleartype');
  }

  // Send X-UA-Compatible HTTP header to force IE to use the most recent
  // rendering engine or use Chrome's frame rendering engine if available.
  if (theme_get_setting('root_chrome_edge') && drupal_get_http_header('X-UA-Compatible') === NULL) {
    drupal_add_http_header('X-UA-Compatible', 'IE=edge,chrome=1');
  }
}
