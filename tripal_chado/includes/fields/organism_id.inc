<?php

/**
 *
 * @param $entity_type
 * @param $entity
 * @param $field
 * @param $instance
 * @param $langcode
 * @param $items
 * @param $display
 * @return string
 */
function tripal_chado_organism_select_formatter(&$element, $entity_type, $entity, $field,
    $instance, $langcode, $items, $display) {
  foreach ($items as $delta => $item) {
    $organism = chado_select_record('organism', array('genus', 'species'), array('organism_id' => $item['value']));
    $content = '<i>' . $organism[0]->genus .' ' . $organism[0]->species . '</i>';
    $element[$delta] = array(
      '#type' => 'markup',
      '#markup' => $content,
    );
  }
}
/**
 *
 * @param $field_name
 * @param $widget
 * @param $form
 * @param $form_state
 * @param $field
 * @param $instance
 * @param $langcode
 * @param $items
 * @param $delta
 * @param $element
 */
function tripal_chado_organism_select_widget(&$widget, $form, $form_state, $field, $instance, $langcode, $items, $delta, $element) {
  $options = tripal_get_organism_select_options(FALSE);
  $widget['value'] = array(
    '#type' => 'select',
    '#title' => $element['#title'],
    '#description' => $element['#description'],
    '#options' => $options,
    '#default_value' => count($items) > 0 ? $items[0]['value'] : 0,
    '#required' => $element['#required'],
    '#weight' => isset($element['#weight']) ? $element['#weight'] : 0,
    '#delta' => $delta,
    '#element_validate' => array('tripal_chado_organism_select_widget_validate'),
  );
  $widget['add_organism'] = array(
    '#type' => 'item',
    '#markup' => l('Add a new species', 'admin/content/bio_data/add/species', array('attributes' => array('target' => '_blank'))),
  );
}
/**
 * Callback function for validating the tripal_chado_organism_select_widget.
 */
function tripal_chado_organism_select_widget_validate($element, &$form_state) {
  $field_name = $element['#parents'][0];

  // If the form ID is field_ui_field_edit_form, then the user is editing the
  // field's values in the manage fields form of Drupal.  We don't want
  // to validate it as if it were being used in a data entry form.
  if ($form_state['build_info']['form_id'] =='field_ui_field_edit_form') {
    return;
  }
  $organism_id = tripal_chado_get_field_form_values($field_name, $form_state);

  if (!$organism_id) {
    form_error($element, t("Please specify an organism."));
  }
}

function organism_id_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  dpm('yo');

  $summary = '';

  if ($display['type'] == 'text_trimmed' || $display['type'] == 'text_summary_or_trimmed') {
    $summary = t('Length: @chars chars', array('@chars' => $settings['trim_length']));
  }

  return $summary;
}
/**
 * Implements hook_field_formatter_settings_form()
 *
 * @param $field
 * @param $instance
 * @param $view_mode
 * @param $form
 * @param $form_state
 */
function organism_id_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  dpm('hi');
  $element = array();

  if ($display['type'] == 'text_trimmed' || $display['type'] == 'text_summary_or_trimmed') {
    $element['trim_length'] = array(
      '#title' => t('Length'),
      '#type' => 'textfield',
      '#size' => 20,
      '#default_value' => $settings['trim_length'],
      '#element_validate' => array('element_validate_integer_positive'),
      '#required' => TRUE,
    );
  }

  return $element;

}

