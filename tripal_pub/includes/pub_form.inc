<?php 

/**
 * This is the chado_pub node form callback. The arguments
 * are out of order from a typical form because it's a defined callback 
 */

function chado_pub_form($node, $form_state) { 
  tripal_core_ahah_init_form();
  $form = array();
  
  $pub = $node->pub;  
  $pub_id = $pub->pub_id;
  
  $d_title        = $form_state['values']['title']        ? $form_state['values']['title']       : $node->title;
  $d_uniquename   = $form_state['values']['uniquename']   ? $form_state['values']['uniquename']  : $pub->uniquename;
  $d_type_id      = $form_state['values']['type_id']      ? $form_state['values']['type_id']     : $pub->type_id->cvterm_id;
  $d_volume       = $form_state['values']['volume']       ? $form_state['values']['volume']      : $pub->volume;
  $d_volumetitle  = $form_state['values']['volumetitle']  ? $form_state['values']['volumetitle'] : $pub->volumetitle;
  $d_series_name  = $form_state['values']['series_name']  ? $form_state['values']['series_name'] : $pub->series_name;
  $d_issue        = $form_state['values']['issue']        ? $form_state['values']['issue']       : $pub->issue;
  $d_pyear        = $form_state['values']['pyear']        ? $form_state['values']['pyear']       : $pub->pyear;
  $d_pages        = $form_state['values']['pages']        ? $form_state['values']['pages']       : $pub->pages;
  $d_miniref      = $form_state['values']['miniref']      ? $form_state['values']['miniref']     : $pub->miniref;
  $d_publisher    = $form_state['values']['publisher']    ? $form_state['values']['publisher']   : $pub->publisher;
  $d_pubplace     = $form_state['values']['pubplace']     ? $form_state['values']['pubplace']    : $pub->pubplace;
  $d_is_obsolete  = $form_state['values']['is_obsolete']  ? $form_state['values']['is_obsolete'] : $pub->is_obsolete;
  
  // if the obsolete value is set by the database then it is in the form of
  // 't' or 'f', we need to convert to 1 or 0
  $d_is_obsolete = $d_is_obsolete == 't' ? 1 : $d_is_obsolete;
  $d_is_obsolete = $d_is_obsolete == 'f' ? 0 : $d_is_obsolete;

  // on AHAH callbacks we want to keep a list of all the properties that have been removed 
  // we'll store this info in a hidden field and retrieve it here
  $d_removed = $form_state['values']['removed']; 
  
  // get the defaults first from the database and then from the form_state
  $default_type = $pub->type_id->cvterm_id;
  
  // get the number of new fields that have been aded via AHAH callbacks
  $num_new = $form_state['values']['num_new'] ? $form_state['values']['num_new'] : 0;

  // initialze default properties array. This is where we store the property defaults
  $d_properties = array();      
  
  // get the list of publication types.  In the Tripal publication
  // ontologies these are all grouped under the term 'Publication Type'
  // we want the default to be 'Journal Article'
  $sql = "
    SELECT CVTS.cvterm_id, CVTS.name
    FROM {cvtermpath} CVTP
      INNER JOIN {cvterm} CVTS ON CVTP.subject_id = CVTS.cvterm_id
      INNER JOIN {cvterm} CVTO ON CVTP.object_id = CVTO.cvterm_id
      INNER JOIN {cv} ON CVTO.cv_id = CV.cv_id
    WHERE CV.name = 'tripal_pub' and CVTO.name = 'Publication Type'
    ORDER BY CVTS.name ASC 
  ";
  $results = chado_query($sql);
  $pub_types = array();
  while ($pub_type = db_fetch_object($results)) {
    $pub_types[$pub_type->cvterm_id] = $pub_type->name;
    // if we don't have a default type then set the default to be 'Journal Article'
    if (strcmp($pub_type->name,"Journal Article") == 0 and !$d_type_id) {
      $d_type_id = $pub_type->cvterm_id;
    }
  }
  
  // get publication properties list 
  $properties_select = array();
  $properties_select[] = 'Select a Property';
  $properties_list = array();
  $sql = "
    SELECT CVTS.cvterm_id, CVTS.name, CVTS.definition
    FROM {cvtermpath} CVTP
      INNER JOIN {cvterm} CVTS ON CVTP.subject_id = CVTS.cvterm_id
      INNER JOIN {cvterm} CVTO ON CVTP.object_id = CVTO.cvterm_id
      INNER JOIN {cv} ON CVTO.cv_id = CV.cv_id
    WHERE CV.name = 'tripal_pub' and CVTO.name = 'Publication Details'
    ORDER BY CVTS.name ASC 
  ";
  $prop_types = chado_query($sql); 
  while ($prop = db_fetch_object($prop_types)) {
    // the 'Citation' term is special because it serves
    // both as a property and as the uniquename for the publiation table
    // it is present by default in the initial fields of the form so
    // we don't want it again in the drop-down list
    if ($prop->name != 'Citation') {
      $properties_select[$prop->cvterm_id] = $prop->name;
    }
    $properties_list[$prop->cvterm_id] = $prop;
  }
  
  $form['pub_id'] = array(
    '#type' => 'hidden',
    '#value' => $pub_id,
  );
  
  $form['title'] = array(
    '#type' => 'textarea',
    '#title' => t('Publication Title'),
    '#default_value' => $d_title,
    '#required' => TRUE,
  );
  $form['type_id'] = array(
    '#type' => 'select',
    '#title' => t('Publication Type'),
    '#options' => $pub_types,
    '#required' => TRUE,
    '#default_value' => $d_type_id,
  );
    
  $form['series_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Series Name (e.g. Journal Name)'),
    '#description' => t('Full name of (journal) series.'),
    '#default_value' => $d_series_name,
    '#required' => TRUE,
  );
  $form['pyear'] = array(
    '#type' => 'textfield',
    '#title' => t('Publication Year'),
    '#default_value' => $d_pyear,
    '#required' => TRUE,
    '#size' => 5,
  );
  
  $form['uniquename'] = array(
    '#type' => 'textarea',
    '#title' => t('Citation'),
    '#default_value' => $d_uniquename,
    '#description' => t('All publications must have a unique citation. Please enter the full citation for this publication. 
      For PubMed style citations list 
      the last name of the author followed by initials. Each author should be separated by a comma. Next comes 
      the title, followed by the series title (e.g. journal name), publication date (3 character Month, day, 4 
      digit year), volume, issue and page numbers. You may also use HTML to provide a link in the citation.  
      Below is an example: <pre>Medeiros PM, Ladio AH, Santos AM, Albuquerque UP. <a href="http://www.ncbi.nlm.nih.gov/pubmed/23462414" target="_blank">Does the selection of medicinal plants by Brazilian local populations 
        suffer taxonomic influence?</a> J Ethnopharmacol. 2013 Apr 19; 146(3):842-52. PubMed PMID: 23462414</pre>'),
    '#required' => TRUE,
  );
  
  // add in the properties that are actually stored in the pub table fields.
  $num_properties = chado_pub_node_form_add_pub_table_props($form, $form_state, $properties_list, 
    $d_properties, $d_removed, $d_volume, $d_volumetitle, $d_issue, $d_pages);
  
  // add in the properties from the pubprop table   
  $num_properties += chado_pub_node_form_add_pubprop_table_props($form, $form_state, $pub_id, $d_properties, $d_removed);
    
  // add in any new properties that have been added by the user through an AHAH callback  
  $num_properties += chado_pub_node_form_add_new_props($form, $form_state, $num_new, $d_properties, $d_removed);
  
  // add an empty row of field to allow for addition of a new property
  chado_pub_node_form_add_new_empty_props($form, $properties_select);
  
  
  $form['removed'] = array(
    '#type' => 'hidden',
    '#value' => $d_removed,
  );

  $form['num_new'] = array(
    '#type' => 'hidden',
    '#value' => $num_new,
  );
  
  $form['is_obsolete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Is Obsolete? (Check for Yes)'),
    '#required' => TRUE,
    '#default_value' => $d_is_obsolete,
  );
  return $form;

}
/*
 * 
 */
function chado_pub_node_form_add_new_empty_props(&$form, $properties_select) {
  
  // add one more blank set of property fields
  $form['properties']['new']["new_id"] = array(
    '#type'          => 'select',
    '#options'       => $properties_select,
  );
  $form['properties']['new']["new_value"] = array(
    '#type'          => 'textarea',
    '#default_value' => '',
    '#cols'          => 5,
    '#rows'          => $rows
  );
  $form['properties']['new']["add"] = array(
    '#type'         => 'image_button',      
    '#value'        => t('Add'),
    '#src'          => drupal_get_path('theme', 'tripal') . '/images/add.png',
    '#ahah' => array(
      'path'    => "tripal_pub/properties/add",
      'wrapper' => 'tripal-pub-edit-properties-table',
      'event'   => 'click',
      'method'  => 'replace',          
    ),   
    '#attributes' => array('onClick' => 'return false;'),
  );   
}
/*
 * 
 */
function chado_pub_node_form_add_new_props(&$form, $form_state, $num_new, &$d_properties, &$d_removed) {
   
  // first, add in all of the new properties that were added through a previous AHAH callback 
  $j = 0;
  $num_properties++;

  // we need to find the
  foreach ($form_state['values'] as $element_name => $value) {
    if (preg_match('/new_value-(\d+)-(\d+)/', $element_name, $matches)) {
      $new_id = $matches[1];
      $rank = $matches[2];
      
      // get this new_id information
      $cvterm = tripal_core_chado_select('cvterm', array('name', 'definition'), array('cvterm_id' => $new_id));
      
      // add it to the $d_properties array
      $d_properties[$new_id][$rank]['name']  = $cvterm->name;
      $d_properties[$new_id][$rank]['id']    = $new_id;
      $d_properties[$new_id][$rank]['value'] = $value;
      $d_properties[$new_id][$rank]['definition']  = $cvterm->definition;
      $num_properties++; 
    
      // determine how many rows we need in the textarea
      $rows = 1;
      if (preg_match('/Abstract/', $cvterm[0]->name)) {
        $rows = 10;
      }
      
      // adjust the term description if needed
      $description = $cvterm[0]->definition;
      if ($cvterm[0]->name == 'Author List') {
        $description .= ' For PubMed style citations list each author with the last name first, followed by initials. Each author should be separated by a comma.';
        $rows = 2;
      }
      
      // add the new fields
      $form['properties']['new'][$new_id][$rank]["new_id-$new_id-$rank"] = array(
        '#type'          => 'item',
        '#value'         => $cvterm[0]->name
      );
      $form['properties']['new'][$new_id][$rank]["new_value-$new_id-$rank"] = array(
        '#type'          => 'textarea',
        '#default_value' => $value,
        '#cols'          => 50,
        '#rows'          => $rows,
        '#description'   => $description,
      );
      
      $form['properties']['new'][$new_id][$rank]["remove-$new_id-$rank"] = array(
        '#type'         => 'image_button',
        '#value'        => t('Remove'),
        '#src'          => drupal_get_path('theme', 'tripal') . '/images/minus.png',
        '#ahah' => array(
          'path'    => "tripal_pub/properties/minus/$new_id/$rank",
          'wrapper' => 'tripal-pub-edit-properties-table',
          'event'   => 'click',
          'method'  => 'replace',
        ), 
        '#attributes' => array('onClick' => 'return false;'),
      );  
    }           
  }
  
  
  // second add in any new properties added during this callback
  if($form_state['post']['add']) {
    $new_id = $form_state['values']['new_id'];
    $new_value = $form_state['values']['new_value'];
    
    // get the rank by counting the number of entries 
    $rank = count($d_properties[$new_id]);
    
    // get this new_id information
    $cvterm = tripal_core_chado_select('cvterm', array('name', 'definition'), array('cvterm_id' => $new_id));
    
    // add it to the $d_properties array
    $d_properties[$new_id][$rank]['name']  = $cvterm->name;
    $d_properties[$new_id][$rank]['id']    = $new_id;
    $d_properties[$new_id][$rank]['value'] = $value;
    $d_properties[$new_id][$rank]['definition']  = $cvterm->definition;
    $num_properties++; 
  
    // determine how many rows we need in the textarea
    $rows = 1;
    if (preg_match('/Abstract/', $cvterm[0]->name)) {
      $rows = 10;
    }
    
    // adjust the term description if needed
    $description = $cvterm[0]->definition;
    if ($cvterm[0]->name == 'Author List') {
      $description .= ' For PubMed style citations list each author with the last name first, followed by initials. Each author should be separated by a comma.';
      $rows = 2;
    }
    
    // add the new fields
    $form['properties']['new'][$new_id][$rank]["new_id-$new_id-$rank"] = array(
      '#type'          => 'item',
      '#value'         => $cvterm[0]->name
    );
    $form['properties']['new'][$new_id][$rank]["new_value-$new_id-$rank"] = array(
      '#type'          => 'textarea',
      '#default_value' => $new_value,
      '#cols'          => 50,
      '#rows'          => $rows,
      '#description'   => $description,
    );
    
    $form['properties']['new'][$new_id][$rank]["remove-$new_id-$rank"] = array(
      '#type'         => 'image_button',
      '#value'        => t('Remove'),
      '#src'          => drupal_get_path('theme', 'tripal') . '/images/minus.png',
      '#ahah' => array(
        'path'    => "tripal_pub/properties/minus/$new_id/$rank",
        'wrapper' => 'tripal-pub-edit-properties-table',
        'event'   => 'click',
        'method'  => 'replace',
      ), 
      '#attributes' => array('onClick' => 'return false;'),
    );
    
  }
    
  return $num_properties;
}
/*
 * 
 */
function chado_pub_node_form_add_pubprop_table_props(&$form, $form_state, $pub_id, &$d_properties, &$d_removed) {
  
  // get the properties for this publication
  $num_properties = 0;
    
  if(!$pub_id) {
    return $num_properties; 
  }
  
  $sql = "
    SELECT CVT.cvterm_id, CVT.name, CVT.definition, PP.value, PP.rank
    FROM {pubprop} PP
      INNER JOIN {cvterm} CVT on CVT.cvterm_id = PP.type_id
    WHERE PP.pub_id = %d
    ORDER BY CVT.name, PP.rank
  ";
  $pub_props = chado_query($sql, $pub_id);
  while ($prop = db_fetch_object($pub_props)) {
    
    $type_id = $prop->cvterm_id;
    $rank = count($d_properties[$type_id]);
    
    // skip properties that were handled above
    if($prop->name == "Volume" or $prop->name == "Volume Title" or 
       $prop->name == "Issue"  or $prop->name == "Pages" or
       $prop->name == "Citation") {
         continue;
    }
    
    // skip any properties that the user requested to delete through a previous
    // AHAH callback or through the current AHAH callback
    if($d_removed[$type_id . '-' . $rank]) {
      continue;
    }
    if($form_state['post']['remove-' . $type_id . '-' . $rank]) {
      $d_removed[$type_id . '-0'] = 1;
      continue;
    }
    
    $d_properties[$type_id][$rank]['name']  = $prop->name;
    $d_properties[$type_id][$rank]['id']    = $type_id;
    $d_properties[$type_id][$rank]['value'] = $prop->value;
    $d_properties[$type_id][$rank]['definition']  = $prop->definition;
    $num_properties++; 
  
    // determine how many rows we need in the textarea
    $rows = 1;
    if (preg_match('/Abstract/', $prop->name)) {
      $rows = 10;
    }
  
    // adjust the term description if needed
    $description = $prop->definition;
    if ($prop->name == 'Author List') {
      $description .= ' For PubMed style citations list each author with the last name first, followed by 
        initials. Each author should be separated by a comma.';
      $rows = 2;
    }
  
    $form['properties'][$type_id][$rank]["prop_id-$type_id-$rank"] = array(
      '#type'          => 'item',
      '#value'         => $prop->name,
    );
    $form['properties'][$type_id][$rank]["prop_value-$type_id-$rank"] = array(
      '#type'          => 'textarea',
      '#default_value' => $prop->value,
      '#cols'          => 50,
      '#rows'          => $rows,
      '#description'   => $description,
    );
    
    $form['properties'][$type_id][$rank]["remove-$type_id-$rank"] = array(
      '#type'         => 'image_button',
      '#value'        => t('Remove'),
      '#src'          => drupal_get_path('theme', 'tripal') . '/images/minus.png',
      '#ahah' => array(
        'path'    => "tripal_pub/properties/minus/$type_id/$rank",
        'wrapper' => 'tripal-pub-edit-properties-table',
        'event'   => 'click',
        'method'  => 'replace',
      ), 
      '#attributes' => array('onClick' => 'return false;'),
    );      
  }
  return $num_properties;
}
/*
 * 
 */
function chado_pub_node_form_add_pub_table_props(&$form, $form_state, $properties_list,
    &$d_properties, &$d_removed, $d_volume, $d_volumetitle, $d_issue, $d_pages) {
      
  $num_properties = 0;
  $rank = 0;
  
  // add properties that are actually part of the pub table  
  foreach($properties_list as $type_id => $prop) {     

    // skip any properties that the user requested to delete through a previous
    // AHAH callback or through the current AHAH callback
    if($d_removed["$type_id-$rank"]) {
      continue;
    }
    if($form_state['post']["remove-$type_id-$rank"]) {
      $d_removed["$type_id-$rank"] = 1;
      continue;
    }
        
    // if any of the properties match the fields in the pub table then we want to include those 
    // automatically
    if (($prop->name == 'Volume' and $d_volume) or         
        ($prop->name == 'Issue' and $d_issue) or 
        ($prop->name == 'Pages' and $d_pages) or
        ($prop->name == 'Volume Title' and $d_volumetitle)) {
          
      $d_properties[$type_id][$rank]['name']  = $prop->name;           
      $d_properties[$type_id][$rank]['id']    = $type_id;            
      $d_properties[$type_id][$rank]['definition']  = $prop->definition;
      $num_properties++;
      
      if ($prop->name == 'Volume') {
        $d_properties[$type_id][$rank]['value'] = $d_volume;
      }
      if ($prop->name == 'Issue') {
        $d_properties[$type_id][$rank]['value'] = $d_issue;
      }
      if ($prop->name == 'Pages') {
        $d_properties[$type_id][$rank]['value'] = $d_pages;
      }
      if ($prop->name == 'Volume Title') {
        $d_properties[$type_id][$rank]['value'] = $d_volumetitle;
      }
      
      // determine how many rows we need in the textarea
      $rows = 1;
      if (preg_match('/Abstract/', $prop->name)) {
        $rows = 10;
      }
      
      // adjust the term description if needed
      $description = $prop->definition;
      if ($prop->name == 'Author List') {
        $description .= ' For PubMed style citations list each author with the last name first, followed by initials. Each author should be separated by a comma.';
        $rows = 2;
      }
      
      // add in the fields
      $form['properties'][$type_id][$rank]["prop_id-$type_id-$rank"] = array(
        '#type'          => 'item',
        '#value'         => $prop->name
      );
      $form['properties'][$type_id][$rank]["prop_value-$type_id-$rank"] = array(
        '#type'          => 'textarea',
        '#default_value' => $d_properties[$type_id][$rank]['value'],
        '#cols'          => 50,
        '#rows'          => $rows,
        '#description'   => $description,
      );
      
      $form['properties'][$type_id][$rank]["remove-$type_id-$rank"] = array(
        '#type'         => 'image_button',
        '#value'        => t('Remove'),
        '#src'          => drupal_get_path('theme', 'tripal') . '/images/minus.png',
        '#ahah' => array(
          'path'    => "tripal_pub/properties/minus/$type_id/$rank",
          'wrapper' => 'tripal-pub-edit-properties-table',
          'event'   => 'click',
          'method'  => 'replace',
        ), 
        '#attributes' => array('onClick' => 'return false;'),
      ); 
    }
  }
  return $num_properties;
}
/*
 * 
 */
function theme_chado_pub_node_form($form) {

  $properties_table = tripal_pub_theme_node_form_properties($form);
  
  $markup  = drupal_render($form['pub_id']);
  $markup .= drupal_render($form['title']);
  $markup .= drupal_render($form['type_id']);
  $markup .= drupal_render($form['series_name']);
  $markup .= drupal_render($form['pyear']);
  $markup .= drupal_render($form['uniquename']);
  $markup .= "<b>Include Additional Details</b><br>You may add additional properties to this publication by scrolling to the bottom of this table, selecting a property type from the dropdown and adding text.  You may add as many properties as desired by clicking the plus button on the right.  To remove a property, click the minus button";
  $markup .= $properties_table;
  $markup .= drupal_render($form['is_obsolete']);
  
  $form['properties'] = array(
    '#type' => 'markup',
    '#value' =>  $markup,
  );
  return drupal_render($form);
}

/*
 * 
 */
function tripal_pub_theme_node_form_properties($form) {
  $rows = array();
  
  if ($form['properties']) {
    
    // first add in the properties derived from the pub and pubprop tables 
    // the array tree for these properties looks like this:
    // $form['properties'][$type_id][$rank]["prop_id-$type_id-$rank"]
    foreach ($form['properties'] as $type_id => $elements) {
      // there are other fields in the properties array so we only
      // want the numeric ones those are our type_id
      if (is_numeric($type_id)) {     
        foreach ($elements as $rank => $element) {
          if (is_numeric($rank)) {          
            $rows[] = array(  
              drupal_render($element["prop_id-$type_id-$rank"]),    
              drupal_render($element["prop_value-$type_id-$rank"]),
              drupal_render($element["remove-$type_id-$rank"]),
            );
          }
        }
      }
    }  

    // second, add in any new properties added by the user through AHAH callbacks
    // the array tree for these properties looks like this: 
    // $form['properties']['new'][$type_id][$rank]["new_id-$new_id-$rank"]
    foreach ($form['properties']['new'] as $type_id => $elements) {
      if (is_numeric($type_id)) {    
        foreach ($elements as $rank => $element) {
          if (is_numeric($rank)) {          
            $rows[] = array(  
              drupal_render($element["new_id-$type_id-$rank"]),    
              drupal_render($element["new_value-$type_id-$rank"]),
              drupal_render($element["remove-$type_id-$rank"]),
            );
          }
        }
      }
    }
  
    // finally add in a set of blank field for adding a new property
    $rows[] = array(
      drupal_render($form['properties']['new']['new_id']),     
      drupal_render($form['properties']['new']['new_value']),
      drupal_render($form['properties']['new']['add']), 
    ); 
  }
  
  $headers = array('Property Type','Value', '');      
  return theme('table', $headers, $rows, array('id'=> "tripal-pub-edit-properties-table"));
}

/*
 * 
 */
function tripal_pub_property_add() {
  $status = TRUE;

  // prepare and render the form
  $form = tripal_core_ahah_prepare_form(); 

  // we only want to return the properties as that's all we'll replace with this AHAh callback
  $data = tripal_pub_theme_node_form_properties($form);
  
  // bind javascript events to the new objects that will be returned 
  // so that AHAH enabled elements will work.
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status, 
      'data'     => $data,
      'settings' => $settings,
    )  
  );
}
/*
 * 
 */
function tripal_pub_property_delete() {
  $status = TRUE;

  // prepare and render the form
  $form = tripal_core_ahah_prepare_form(); 

  // we only want to return the properties as that's all we'll replace with this AHAh callback
  $data = tripal_pub_theme_node_form_properties($form);

  // bind javascript events to the new objects that will be returned 
  // so that AHAH enabled elements will work.
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status, 
      'data'     => $data,
      'settings' => $settings,
    )  
  );
}