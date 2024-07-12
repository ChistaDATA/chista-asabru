function set_fields(tag, timestamp, record)
      record['tag'] = tag
      record['timestamp'] = timestamp
      record['data'] = record['log']
      return tag, timestamp, record
end