<?php

/**
 * Обновление курса валют с ресурса http://www.cbr.ru/scripts/XML_daily.asp
 */
function custom_exchange_rates_update_rate() {

    $last_rate_array = custom_exchange_rates_get_last_rate_array_from_source();

    custom_exchange_rates_save($last_rate_array);

}


/**
 * Записывает последний актуальный курс валют в базу данных
 *
 * @param array $last_rate_date - массив курса валют
 */
function custom_exchange_rates_save($last_rate_date) {

    // Проверка и запись времени курса валют

    $last_rate_time = custom_exchange_rates_get_time_from_date($last_rate_date['@attributes']['Date']);

    $last_rate_date_id = custom_exchange_rates_check_date($last_rate_time);

    if (!empty($last_rate_date_id))
        drupal_goto("admin/custom_exchange_rates");

    $save_date = custom_exchange_rates_save_date($last_rate_time);

    // Проверка и запись валют

    foreach ($last_rate_date['Valute'] as $id => $valute) {

        $rate_id = NULL;

        $valute_id = $valute['@attributes']['ID'];

        $currency_id = custom_exchange_rates_check_currency($valute_id);

        $currency = new stdClass();

        if (!empty($currency_id))
            $currency->currency_id = $currency_id;

        $currency->currency_valute_id = $valute_id;
        $currency->currency_num_code = $valute['NumCode'];
        $currency->currency_char_code = $valute['CharCode'];
        $currency->currency_nominal = $valute['Nominal'];
        $currency->currency_name = $valute['Name'];

        custom_exchange_rates_save_currency($currency);

        // Проверка и запись значения валют

        $rate_id = custom_exchange_rates_check_rate($currency_id, $save_date->date_id);

        if (!empty($rate_id))
            continue;

        $rate = new stdClass();
        $rate->currency_id = $currency->currency_id;
        $rate->date_id = $save_date->date_id;

        $valute_value = str_replace(',','.', $valute['Value']);
        $rate->currency_value = $valute_value;

        custom_exchange_rates_save_rate($rate);

     }

    drupal_goto("admin/custom_exchange_rates");

}


/**
 * Возвращает массив данных последнего актуального курса валют с ресурса http://www.cbr.ru/scripts/XML_daily.asp
 *
 * @return array $last_rate_array - массив курса валют
 */
function custom_exchange_rates_get_last_rate_array_from_source() {

    $requestUrl = 'http://www.cbr.ru/scripts/XML_daily.asp';
    $xml = simplexml_load_file($requestUrl);

    $json_string = json_encode($xml);
    $last_rate_array = json_decode($json_string, TRUE);

    return $last_rate_array;

}


/**
 * Возвращает временную метку по дате формата d.m.Y
 *
 * @param string $date - Дата формата d.m.Y
 * @return int timestamp - Временная метка
 */
function custom_exchange_rates_get_time_from_date($date){

    list($day, $month, $year) = explode('.', $date);
    $time =  mktime(0, 0, 0, $month, $day, $year);

    return $time;

}


/**
 * Возвращает идентификатор даты обновления курса валют по временной метке
 *
 * @param int $time - временная метка
 * @return int $date_id - идентификатор даты курса валют
 */
function custom_exchange_rates_check_date($time){

    $query = db_select('custom_exchange_rates_date', 'rates');
    $query->condition('rates.date_value', $time);
    $query->fields('rates', array('date_id'));
    $date_id = $query->execute()->fetchField();

    return $date_id;

}


/**
 * Сохранение даты последнего обновления курса валют
 *
 * @param int $time - временная метка курса валют
 * @return object $rates_date - даты курса валют
 */
function custom_exchange_rates_save_date($time) {

    $rates_date = new stdClass();
    $rates_date->date_value = $time;

    empty($rates_date->date_id);

    drupal_write_record('custom_exchange_rates_date', $rates_date);

    return $rates_date;

}


/**
 * Возвращает идентификатор валюты по идентификатору Valute ID
 *
 * @param int $currency_valute_id - идентификатор валюты Valute ID
 * @return int currency_id - идентификатор валюты
 */
function custom_exchange_rates_check_currency($currency_valute_id){
    
    $query = db_select('custom_exchange_rates_currency', 'currency');
    $query->condition('currency.currency_valute_id', $currency_valute_id);
    $query->fields('currency', array('currency_id'));
    $currency_id = $query->execute()->fetchField();

    return $currency_id;

}


/**
 * Сохранение объекта валюты
 *
 * @param object $currency - объект валюты
 * @return object $currency - объект валюты
 */
function custom_exchange_rates_save_currency($currency) {

    $new = empty($currency->currency_id);

    if ($new)
        drupal_write_record('custom_exchange_rates_currency', $currency);
    else
        drupal_write_record('custom_exchange_rates_currency', $currency, 'currency_id');

    return $currency;

}


/**
 * Возвращает иденификатор значения валюты по идентификатору даты и валюты
 *
 * @param int $currency_id - Идентификатор валюты
 * @param int $date_id - Идентификатор даты
 *
 * @return int $rate_id - Идентификатор значения валюты
 */
function custom_exchange_rates_check_rate($currency_id, $date_id){

    $query = db_select('custom_exchange_rates_rate', 'rate');
    $query->condition('rate.currency_id', $currency_id);
    $query->condition('rate.date_id', $date_id);
    $query->fields('rate', array('rate_id'));
    $rate_id = $query->execute()->fetchField();

    return $rate_id;

}


/**
 * Сохранение значение валюты
 *
 * @param object $rate - объект значения валюты
 * @return object $rate - объект значения валюты
 */
function custom_exchange_rates_save_rate($rate) {

    drupal_write_record('custom_exchange_rates_rate', $rate);

    return $rate;

}