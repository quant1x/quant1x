package std

import (
	"reflect"
	"testing"
)

func TestFromHexString(t *testing.T) {
	type args struct {
		s string
	}
	tests := []struct {
		name    string
		args    args
		want    []byte
		wantErr bool
	}{
		{
			name:    "t1",
			args:    args{s: "30"},
			want:    []byte{0x30},
			wantErr: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := FromHexString(tt.args.s)
			if (err != nil) != tt.wantErr {
				t.Errorf("FromHexString() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if !reflect.DeepEqual(got, tt.want) {
				t.Errorf("FromHexString() got = %v, want %v", got, tt.want)
			}
		})
	}
}
