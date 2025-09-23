package chipdistribution

import (
	"reflect"
	"testing"
)

func Test_generatePriceGrid(t *testing.T) {
	type args struct {
		low    float64
		high   float64
		step   float64
		digits int
	}
	tests := []struct {
		name string
		args args
		want []float64
	}{
		{
			name: "generate price grid",
			args: args{
				low:    1,
				high:   2,
				step:   0.01,
				digits: 2,
			},
			want: generatePriceGrid(0, 0, 0, 0),
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := generatePriceGrid(tt.args.low, tt.args.high, tt.args.step, tt.args.digits); !reflect.DeepEqual(got, tt.want) {
				t.Errorf("generatePriceGrid() = %v, want %v", got, tt.want)
			}
		})
	}
}
