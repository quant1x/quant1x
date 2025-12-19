package std

import "testing"

func TestHighestOneBit(t *testing.T) {
	type args struct {
		x uint64
	}
	tests := []struct {
		name string
		args args
		want uint64
	}{
		{
			name: "0",
			args: args{
				x: 0,
			},
			want: 1 << 63,
		},
		{
			name: "1",
			args: args{
				x: 1,
			},
			want: 1,
		},
		{
			name: "2",
			args: args{
				x: 2,
			},
			want: 2,
		},
		{
			name: "3",
			args: args{
				x: 3,
			},
			want: 4,
		},
		{
			name: "5",
			args: args{
				x: 5,
			},
			want: 8,
		},
		{
			name: "1023",
			args: args{
				x: 1023,
			},
			want: 1024,
		},
		{
			name: "1<<63",
			args: args{
				x: 1 << 63,
			},
			want: 1 << 63,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := HighestOneBit(tt.args.x); got != tt.want {
				t.Errorf("HighestOneBit() = %v, want %v", got, tt.want)
			}
		})
	}
}

func Test_highestOneBit(t *testing.T) {
	type args struct {
		x uint64
	}
	tests := []struct {
		name string
		args args
		want uint64
	}{
		{
			name: "0",
			args: args{
				x: 0,
			},
			want: 1 << 63,
		},
		{
			name: "1",
			args: args{
				x: 1,
			},
			want: 1,
		},
		{
			name: "2",
			args: args{
				x: 2,
			},
			want: 2,
		},
		{
			name: "3",
			args: args{
				x: 3,
			},
			want: 4,
		},
		{
			name: "5",
			args: args{
				x: 5,
			},
			want: 8,
		},
		{
			name: "1023",
			args: args{
				x: 1023,
			},
			want: 1024,
		},
		{
			name: "1<<63",
			args: args{
				x: 1 << 63,
			},
			want: 1 << 63,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := highestOneBit(tt.args.x); got != tt.want {
				t.Errorf("highestOneBit() = %v, want %v", got, tt.want)
			}
		})
	}
}
