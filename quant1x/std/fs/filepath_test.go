package fs

import "testing"

func TestFilepath(t *testing.T) {
	_ = DefaultDirMode
	_ = DefaultFileMode
}

func TestEnsureFileDirExists(t *testing.T) {
	type args struct {
		filePath string
	}
	tests := []struct {
		name    string
		args    args
		wantErr bool
	}{
		{
			name: "EnsureFileDirExists",
			args: args{
				filePath: "./testdata/",
			},
			wantErr: false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if err := EnsureFileDirExists(tt.args.filePath); (err != nil) != tt.wantErr {
				t.Errorf("EnsureFileDirExists() error = %v, wantErr %v", err, tt.wantErr)
			}
		})
	}
}
