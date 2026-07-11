package mem

import "testing"

func TestAlign(t *testing.T) {
	type args struct {
		n int
	}
	tests := []struct {
		name string
		args args
		want int
	}{
		{
			name: "align-1",
			args: args{
				n: 1,
			},
			want: 4096,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := Align(tt.args.n); got != tt.want {
				t.Errorf("Align() = %v, want %v", got, tt.want)
			}
		})
	}
}
