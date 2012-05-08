<?php

/**
 * @file
 * Contains the theme settings form elements for the development extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_development_theme_settings_form_alter($element, &$form, $form_state) {
  $element['root_rebuild_theme_registry'] = array(
    '#type' => 'checkbox',
    '#title' => t('Rebuild the theme registry on every page load'),
    '#description' => t('While creating new templates and theme overrides the theme registry needs to be rebuilt. Note: This has a high (negative) performance impact and has to be deactivated once your website goes into production.'),
    '#default_value' => theme_get_setting('root_rebuild_theme_registry'),
  );

  $element['root_media_query_widget'] = array(
    '#type' => 'checkbox',
    '#title' => t('Media query widget'),
    '#description' => t('Adds a toolbar at the bottom of the browser window that displays the current width of the viewport and can be used to generate media query strings.'),
    '#default_value' => theme_get_setting('root_media_query_widget'),
  );

  return $element;
}
