<?php

/**
 * Возвращает курс валют в виде файла с массивом данных в формате json
 */
function custom_exchange_rates_json() {

    $rows = custom_exchange_rates_report_get_rows();

    $result = array();
    $result['result'] = $rows;
    $json_result = json_encode($result);

    $current_date = date('d.m.Y', custom_exchange_rates_report_get_current_date());

    header('Content-disposition: attachment; filename=exchange_rates_' . $current_date . '.json');
    header('Content-type: application/json');

    echo($json_result);

}


/**
 * Формирует массив данных для отчета по парамтерам фильтрации
 *
 * @return array $rows - массив данных для отчета
 */
function custom_exchange_rates_report_get_rows() {

    // Получаем данные согласно GET параметрам
    $rates = custom_exchange_rates_report_get_data();

    $current_date = custom_exchange_rates_report_get_current_date();

    // Формируем данные для таблицы
    $rows = array();
    $rows['exchange_date'] = $current_date;
    foreach ($rates as $rate) {

        $rows['exchange_rates'][] = array(
            'Valute ID' => $rate->currency_valute_id,
            'Num Code' => $rate->currency_num_code,
            'Char Code' => $rate->currency_char_code,
            'Nominal' => $rate->currency_nominal,
            'Name' => $rate->currency_name,
            'Value' => $rate->currency_value,
        );

    }

    return $rows;

}


/**
 * Возвращает последнюю временную метку из базы или заданную временную метку в фильтре
 *
 * @return int $current_date - временная метка
 */
function custom_exchange_rates_report_get_current_date() {

    $last_update_timestamp = custom_exchange_rates_get_last_update_timestamp();

    if (isset($_GET['date']))
        $current_date = $_GET['date'];
    else
        $current_date = $last_update_timestamp;

    return $current_date;

}


/**
 * Получение данных для формирования отчета
 *
 * @return array $exchange_rates - Массив объектов курса валют. Ключ элемента массива - currency_id. Объект курса валют содержит следующие данные:
 * * int currency_id - идентификатор валюты
 * * int currency_valute_id - Valute ID
 * * string currency_num_code - Num Code
 * * string currency_char_code - Char Code
 * * string currency_nominal - Nominal
 * * string currency_name - название валюты
 * * float currency_value - значение валюты
 */
function custom_exchange_rates_report_get_data() {

    $query = db_select('custom_exchange_rates_currency', 'currency');

    $query->innerJoin('custom_exchange_rates_rate', 'rate', 'currency.currency_id = rate.currency_id');
    $query->innerJoin('custom_exchange_rates_date', 'date', 'rate.date_id = date.date_id');

    $current_date = custom_exchange_rates_report_get_current_date();

    $query->condition('date.date_value', $current_date);

    if (isset($_GET['currency']))
        $query->condition('currency.currency_id', $_GET['currency']);

    $query->fields('currency');
    $query->addField('rate', 'currency_value');

    $exchange_rates = $query->execute()->fetchAll();

    return $exchange_rates;

}