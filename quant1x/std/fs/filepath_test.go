package fs

import "testing"

func TestFilepath(t *testing.T) {
	_ = DefaultDirMode
	_ = DefaultFileMode
}

func TestFileDirExists(t *testing.T) {
	type args struct {
		filePath string
	}
	tests := []struct {
		name    string
		args    args
		wantErr bool
	}{
		{
			name: "FileDirExists",
			args: args{
				filePath: "./testdata/",
			},
			wantErr: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if err := FileDirExists(tt.args.filePath); (err != nil) != tt.wantErr {
				t.Errorf("FileDirExists() error = %v, wantErr %v", err, tt.wantErr)
			}
		})
	}
}
