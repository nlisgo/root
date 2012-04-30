<?php

/**
 * @file
 * Please supply a file description.
 */

require_once dirname(__FILE__) . '/root.generator.inc';

/**
 * Implements hook_drush_command().
 */
function root_generator_drush_command() {
  $items = array();

  $items['root-subtheme'] = array(
    'description' => 'Create a Root subtheme.',
    'arguments' => array(
      'name' => 'The name of your subtheme.',
    ),
    'options' => array(
      'destination' => 'The destination of your subtheme. Defaults to "all" (sites/all/themes).',
      'machine_name' => 'The machine-readable name of your subtheme. This will be auto-generated from the human-readable name if ommited.',
      'starterkit' => 'The starterkit that your subtheme should use. It must have the same base theme as your subtheme. Defaults to "starterkit_root_html5" if "--base" is not set either.',
      'enable' => 'Automatically enable the subtheme after creation.',
      'set-default' => 'Automatically enable the subtheme after creation and make it the default theme.',
    ),
    'examples' => array(
      'drush root-subtheme "My Theme"' => 'Creates a Root subtheme called "My Theme".',
      'drush root-subtheme "My Theme" --destination=example.com' => 'Creates a Root subtheme called "My Theme" in sites/example.com/themes.',
      'drush root-subtheme "My Theme" --starterkit=starterkit_root_html5' => 'Uses the HTML5 starterkit to create a Root subtheme called "My Theme" in sites/all/themes (default).',
    ),
  );

  return $items;
}

/**
 * Implements of hook_drush_help().
 */
function root_generator_drush_help($section) {
  switch ($section) {
    case 'drush:root-subtheme':
      return dt('This command will create a Root subtheme.');
  }
}

/**
 * Implements of drush_hook_COMMAND_validate().
 */
function drush_root_generator_root_subtheme_validate($name) {
  system_rebuild_theme_data();

  $themes = list_themes();
  $destination = drush_get_option('destination', 'all');
  $machine_name = drush_get_option('machine_name', _root_generator_transform_theme_name($name));
  $base = drush_get_option('base');
  $starterkit = drush_get_option('starterkit');

  drush_set_option('machine_name', $machine_name);

  if (!_root_generator_theme_exists('alpha') || !_root_generator_theme_exists('root')) {
    return drush_set_error('root_generator_THEME_ERROR', dt('Where is the Root base theme? I has not found it. :('));
  }

  if (!$machine_name || !preg_match('/^[a-z][a-z0-9_]*$/', $machine_name)) {
    return drush_set_error('root_generator_THEME_ERROR', dt('The machine name is invalid or could not be generated properly. It may only contain lowercase numbers, letters and underscores and must start with a letter. Please provide a proper machine name by using "--machine_name".'));
  }

  if (_root_generator_theme_exists($machine_name)) {
    return drush_set_error('root_generator_THEME_ERROR', dt('A theme with that name already exists. The machine-readable name must be unique.'));
  }

  if (!in_array($destination, root_generator_sites())) {
    return drush_set_error('root_generator_THEME_ERROR', dt('The destination is invalid.'));
  }

  if ($starterkit && (!_root_generator_theme_exists($starterkit) || !_root_generator_is_starterkit($starterkit))) {
    return drush_set_error('root_generator_THEME_ERROR', dt('There is no valid starterkit with the name !starterkit.', array('!starterkit' => $starterkit)));
  }

  if (!$base && $starterkit) {
    $base = $themes[$starterkit]->info['base theme'];
    drush_set_option('base', $base);
  }

  if ($base && !array_key_exists($base, root_generator_base_themes())) {
    return drush_set_error('root_generator_THEME_ERROR', dt('There is no base theme with the name !base.', array('!base' => $base)));
  }

  if ($starterkit && $themes[$starterkit]->info['base theme'] != $base) {
    return drush_set_error('root_generator_THEME_ERROR', dt('The base theme of the selected starterkit does not match the selected base theme.'));
  }

  if (!$base && !$starterkit) {
    drush_set_option('base', 'root');
    drush_set_option('starterkit', 'starterkit_root_html5');
  }
  else if (!$base) {
    drush_set_option('base', 'root');
  }
}

/**
 * Implements of drush_hook_COMMAND().
 */
function drush_root_generator_root_subtheme($name) {
  $subtheme = new stdClass();
  $subtheme->new = TRUE;
  $subtheme->name = $name;
  $subtheme->machine_name = drush_get_option('machine_name');
  $subtheme->starterkit = drush_get_option('starterkit');
  $subtheme->path = 'sites/' . drush_get_option('destination', 'all') . '/themes/' . $subtheme->machine_name;
  $subtheme->base = drush_get_option('base');
  $subtheme->default = drush_get_option('set-default') !== NULL;
  $subtheme->status = $subtheme->default || drush_get_option('enable') !== NULL;

  root_generator_subtheme_create($subtheme);
  root_generator_subtheme_process($subtheme);
}
