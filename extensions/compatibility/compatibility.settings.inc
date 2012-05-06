<?php

/**
 * @file
 * Contains the theme settings form elements for the metatags extension.
 */

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_compatibility_theme_settings_form_alter($element, &$form, $form_state) {
  $scales = root_extension_compatibility_scale_options(1, 10, 0.5);

  $element['root_conditional_comments_html'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add conditional classes for Internet Explorer'),
    '#description' => t('Adds conditional classes (for Internet Explorer) to the &lt;html&gt;.'),
    '#default_value' => theme_get_setting('root_conditional_comments_html'),
  );

  $element['root_viewport'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow customizing viewport meta properties on iOS and Android devices'),
    '#description' => t('By default, most mobile browsers choose their own way to handle page rendering by using the viewport meta tag. iOS and Android are "capable" of displaying full size websites by simply scaling down the view. However, when using a truly mobile version of your theme, you will / may want to customize these settings. You can find more information about this in the <a href="http://drupal.org/node/819164">Omega documentation</a>.'),
    '#default_value' => theme_get_setting('root_viewport'),
  );

  $element['root_viewport_settings'] = array(
    '#type' => 'container',
    '#states' => array(
      'visible' => array(
        ':input[name="root_viewport"]' => array('checked' => TRUE),
      ),
    ),
    '#attributes' => array('class' => array('root-settings-container')),
  );

  $element['root_viewport_settings']['root_viewport_initial_scale'] = array(
    '#type' => 'select',
    '#title' => t('Initial scale'),
    '#default_value' => theme_get_setting('root_viewport_initial_scale'),
    '#options' => $scales,
    '#description' => t('The initial scaling of the page. This should almost always be set to 1.0.'),
  );

  $element['root_viewport_settings']['root_viewport_min_scale'] = array(
    '#type' => 'select',
    '#title' => t('Minimum scale'),
    '#default_value' => theme_get_setting('root_viewport_min_scale'),
    '#options' => $scales,
    '#description' => t('The minimum scaling of the site. This should usually be the same as your <em>Initial scale</em> setting.'),
  );

  $element['root_viewport_settings']['root_viewport_max_scale'] = array(
    '#type' => 'select',
    '#title' => t('Maximum scale'),
    '#default_value' => theme_get_setting('root_viewport_max_scale'),
    '#options' => $scales,
    '#description' => t('The maximum scaling of the site. This can be any value between 1 and 10, but should not be too big if you want to preserve your mobile look and feel.'),
  );

  $element['root_viewport_settings']['root_viewport_user_scalable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Scalable by user'),
    '#description' => t('<p>Determine if a user can resize the screen. This is usually accomplished via multi-touch gestures on iOS and Android devices. If your mobile theme is very customized and presented with good typography and graphics for a reduced mobile size, it is recommended to leave this setting unchecked. If it is left unchecked, the min-scale and max-scale properties will be ignored.</p><p class="marker">HTC Android devices do NOT (currently) respect the viewport meta tag for <em>user-scalable</em>. It will render at the appropriate <em>initial-scale</em> set above, but a user can still zoom in/out.</p>'),
    '#default_value' => theme_get_setting('root_viewport_user_scalable'),
  );

  $element['root_chrome_edge'] = array(
    '#type' => 'checkbox',
    '#title' => t("Enable Chrome Edge support for Internet Explorer"),
    '#description' => t('Edge mode tells Internet Explorer to display content in the highest mode available. With Internet Explorer 9, this is equivalent to IE9 mode. If a (hypothetical) future release of Internet Explorer supported a higher compatibility mode, pages set to edge mode would appear in the highest mode supported by that version. Those same pages would still appear in IE9 mode when viewed with Internet Explorer 9.'),
    '#default_value' => theme_get_setting('root_chrome_edge'),
  );

  return $element;
}

/**
 * A helper function for building a float value options array that can be used
 * in a form.
 *
 * @param $start
 *   The number to start with.
 * @param $end
 *   The number to end with.
 * @param $gap
 *   The gap between steps.
 *
 * @return
 *   An array of scale options.
 */
function root_extension_compatibility_scale_options($start, $end, $gap) {
  $options = array();
  foreach (range($start, $end, $gap) as $number) {
    // Format the value to display with one decimal.
    $options[(string) $number] = number_format($number, 1);
  }
  return $options;
}
