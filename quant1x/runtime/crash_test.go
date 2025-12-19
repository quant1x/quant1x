package runtime

import "testing"

func Test_sprintf(t *testing.T) {
	type args struct {
		v []any
	}
	tests := []struct {
		name string
		args args
		want string
	}{
		{
			name: "空",
			args: args{v: []any{}},
			want: "",
		},
		{
			name: "1-int",
			args: args{v: []any{5}},
			want: "5",
		},
		{
			name: "1-string",
			args: args{v: []any{"2"}},
			want: "2",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := sprintf(tt.args.v...); got != tt.want {
				t.Errorf("sprintf() = %v, want %v", got, tt.want)
			}
		})
	}
}
