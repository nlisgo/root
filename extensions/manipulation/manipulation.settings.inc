<?php

/**
 * @file
 * Contains the theme settings form elements for the development extension.
 */

include_once dirname(__FILE__) . '/manipulation.inc';

/**
 * Implements hook_extension_EXTENSION_theme_settings_form_alter().
 */
function root_extension_manipulation_theme_settings_form_alter($element, &$form, $form_state) {
  $element['root_manipulate_css'] = array(
    '#type' => 'fieldset',
    '#title' => t('Manipulate stylesheets and classes'),
    '#collapsible' => TRUE,
  );

  $element['root_manipulate_css']['root_class_rules'] = array(
    '#type' => 'textarea',
    '#title' => t('Class rules'),
    '#description' => t("Enter one class rule per line."),
    // The rules are stored in an array.
    '#default_value' => implode("\n", (array) theme_get_setting('root_class_rules')),
    '#element_validate' => array('root_class_rules_element_validate'),
  );

  $element['root_manipulate_css']['root_css_exclude'] = array(
    '#type' => 'textarea',
    '#title' => t('Exclude CSS files'),
    '#description' => t("Enter one file per line. The '*' character is a wildcard. Each path is relative to a module, theme, profile, theme engine or the 'misc' folder."),
    // The paths are stored in an array.
    '#default_value' => implode("\n", (array) theme_get_setting('root_css_exclude')),
  );

  $element['root_manipulate_js'] = array(
    '#type' => 'fieldset',
    '#title' => t('Manipulate JavasSript files'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $element['root_manipulate_js']['root_js_exclude'] = array(
    '#type' => 'textarea',
    '#title' => t('Exclude JavaScript files'),
    '#description' => t("Enter one file per line. The '*' character is a wildcard. Each path is relative to a module, theme, profile, theme engine or the 'misc' folder."),
    // The paths are stored in an array.
    '#default_value' => implode("\n", (array) theme_get_setting('root_js_exclude')),
  );

  // We need a custom submit handler to store the CSS and JS paths as arrays.
  $form['#submit'][] = 'root_extension_manipulation_theme_settings_form_submit';

  return $element;
}

/**
 * Form submit handler for the manipulation theme settings extension.
 */
function root_extension_manipulation_theme_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  // Explode and trim the values for the CSS and JavaScript exclusion so we end
  // up with a clean array.
  foreach (array('root_css_exclude', 'root_js_exclude', 'root_class_rules') as $item) {
    $exclude = array_filter(array_map('trim', explode("\n", $values[$item])));
    form_set_value(array('#parents' => array($item)), $exclude, $form_state);
  }
}

/**
 * Validation callback for the CSS class rules form element.
 */
function root_class_rules_element_validate($element, &$form_state) {
  $values = array_filter(array_map('trim', explode("\n", $element['#value'])));

  foreach ($values as $rule) {
    if (!preg_match('/^(\+|-)([a-z_]+)(:([a-z_])*)?\[(([a-z\-]+|(\*(?!\*))|\[[a-zA-Z0-9_# ]+(:[a-zA-Z0-9_# ]+)*\])+)\]$/', $rule, $matches)) {
      // The class rule doesn't match the regex pattern.
      form_error($element, t('At least one of te provided class rules is invalid: @rule.', array('@rule' => $rule)), $form_state);
    }
    elseif ($matches[1] == '+' && strpos($rule, '*') !== FALSE) {
      // 'Add' rules don't support wildcards of course.
      form_error($element, t("Wildcards (asterix characters) are not supported by 'add' (+) rules: @rule.", array('@rule' => $rule)), $form_state);
    }
  }
}
