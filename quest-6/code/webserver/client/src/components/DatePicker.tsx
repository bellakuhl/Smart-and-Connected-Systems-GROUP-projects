import 'date-fns';
import * as React from 'react';
import Grid from '@material-ui/core/Grid';
import DateFnsUtils from '@date-io/date-fns';
import {
  MuiPickersUtilsProvider,
  KeyboardDatePicker,
} from '@material-ui/pickers';


interface IDatePickerProps {
  label: string;
  onChange: (date: Date) => void;
  defaultDate: number|null
}

export function DatePicker(props: IDatePickerProps) {
  // The first commit of Material-UI
  let startDate = null;
  if (props.defaultDate !== null) {
    startDate = new Date(props.defaultDate);
  }
  const [selectedDate, setSelectedDate] = React.useState<Date | null>(startDate);

  const handleDateChange = (date: Date | null) => {
    setSelectedDate(date);
    props.onChange(date);
  };

  return (
    <MuiPickersUtilsProvider utils={DateFnsUtils}>
        <KeyboardDatePicker
          margin="normal"
          id="date-picker-dialog"
          label={props.label}
          format="MM/dd/yyyy"
          disableFuture={true}
          value={selectedDate}
          onChange={handleDateChange}
          KeyboardButtonProps={{
            'aria-label': 'change date',
          }}
        />
    </MuiPickersUtilsProvider>
  );
}