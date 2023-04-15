#!/bin/bash

dir=/tmp/cfg
binary=${1}
# xml of json
type=${2}
test=${3}
file=test.${type}
args="--max-respawn 0 --directory ${dir} --config test.${type}"
full=${dir}/${file}

function no_file()
{
  rm -f ${full} 2>/dev/null
}
function create_empty()
{
  echo "" > ${full}
}
function create_no_process()
{
  echo "" > ${full}
  case ${type} in
  "xml") 
    echo "<?xml version=\"1.0\" encoding=\"utf-8\"?><proc></proc>" > ${full}
    ;;
  "json") 
    echo "{\"proc\": {}}" > ${full}
    ;;
  esac
}
function create_empty_name()
{
  echo "" > ${full}
  case ${type} in
  "xml") 
    echo "<?xml version=\"1.0\" encoding=\"utf-8\"?><process name=\"\"></process>" > ${full}
    ;;
  "json") 
    echo "{\"process\": {\"name\": \"\"}}" > ${full}
    ;;
  esac
}
function create_env()
{
  echo "" > ${full}
  case ${type} in
  "xml") 
    echo "<?xml version=\"1.0\" encoding=\"utf-8\"?><process name=\"ls\"><path>/bin</path><args><arg>-a</arg><arg>-l</arg><arg>-s</arg><arg>/home</arg></args><envs><env name=\"wd\" value=\"1\"/></envs></process>" > ${full}
    ;;
  "json") 
    echo "{\"process\": {\"name\": \"ls\",\"path\": \"/bin\",\"args\": [\"-a\",\"-l\",\"-s\",\"/home\"],\"envs\": [{\"name\": \"wd\", \"value\": \"1\"}]}}" > ${full}
    ;;
  esac
}

mkdir -p ${dir}
case ${test} in
  "no_file") no_file ;;
  "empty") create_empty ;;
  "no_process") create_no_process ;;
  "empty_name") create_empty_name ;;
  "env") create_env ;;
  *) 
    echo "Invalid test"
    exit 1
    ;;
esac

echo "${binary} ${args}"
${binary} ${args}
exit ${?}