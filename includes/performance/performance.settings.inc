<?php

/**
 * @file
 * Contains the theme settings form elements for the development extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function omega_extension_performance_theme_settings_form_alter($element, &$form, $form_state) {
  $element['omega_js_footer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Output JavaScript in the footer'),
    '#description' => t('Outputs all JavaScript files and settings in the footer instead of placing them in the header (default).'),
    '#default_value' => theme_get_setting('omega_js_footer'),
  );

  $element['omega_media_queries_inline'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enforce embedded media queries'),
    '#description' => t('Embeds all media queries (and media types) in the CSS files automatically instead of leaving them on the &lt;styles&gt; tags. This potentially reduces the amount of separate CSS aggregates on your website.'),
    '#default_value' => theme_get_setting('omega_media_queries_inline'),
  );

  return $element;
}
