# Transport-Catalog:
Реализована систему хранения транспортных маршрутов и обработки запросов к ней. Сначала на вход подаются запросы на 
создание базы данных, затем — запросы к самой базе.  
Весь ввод и вывод в формате *JSON*.
  
## Формат ввода базы данных
Примеры ввода и ввывода в папке example

* Вещественные числа не должны задаются в экспоненциальной записи, то есть обязательно имеют целую часть и, 
возможно, дробную часть через десятичную точку.
  
* Каждый запрос к базе дополнительно должен иметь идентификатор в поле *id* — целое число от 0 до 2147483647. 
Ответ на запрос содержит идентификатор этого запроса в поле *request_id*. 
Это позволяет выводить ответы на запросы в любом порядке.
  
* Ключи словарей могут располагаться в произвольном порядке. 
Форматирование (то есть пробельные символы вокруг скобок, запятых и двоеточий) 
не имеет значения как во входном, так и в выходном *JSON*.