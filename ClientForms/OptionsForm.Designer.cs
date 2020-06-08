namespace ThorQ
{
	partial class OptionsForm
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(OptionsForm));
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.hostnameBox = new System.Windows.Forms.TextBox();
            this.CheckConnectionButton = new System.Windows.Forms.Button();
            this.OptCancelButton = new System.Windows.Forms.Button();
            this.OkButton = new System.Windows.Forms.Button();
            this.portBox = new System.Windows.Forms.NumericUpDown();
            this.NetworkingBox = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.PortSelector = new System.Windows.Forms.ComboBox();
            this.BaudSelector = new System.Windows.Forms.ComboBox();
            this.CollarPortLabel = new System.Windows.Forms.Label();
            this.CollarBaudLabel = new System.Windows.Forms.Label();
            this.button3 = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.portBox)).BeginInit();
            this.NetworkingBox.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(55, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Hostname";
            this.label1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OptionsForm_MouseDown);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 42);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(26, 13);
            this.label2.TabIndex = 1;
            this.label2.Text = "Port";
            this.label2.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OptionsForm_MouseDown);
            // 
            // hostnameBox
            // 
            this.hostnameBox.Location = new System.Drawing.Point(67, 13);
            this.hostnameBox.Name = "hostnameBox";
            this.hostnameBox.Size = new System.Drawing.Size(100, 20);
            this.hostnameBox.TabIndex = 2;
            this.hostnameBox.TextChanged += new System.EventHandler(this.hostnameBox_TextChanged);
            // 
            // CheckConnectionButton
            // 
            this.CheckConnectionButton.Location = new System.Drawing.Point(6, 65);
            this.CheckConnectionButton.Name = "CheckConnectionButton";
            this.CheckConnectionButton.Size = new System.Drawing.Size(160, 22);
            this.CheckConnectionButton.TabIndex = 4;
            this.CheckConnectionButton.Text = "Check Connection";
            this.CheckConnectionButton.UseVisualStyleBackColor = true;
            this.CheckConnectionButton.Click += new System.EventHandler(this.CheckConnectionButton_Click);
            // 
            // OptCancelButton
            // 
            this.OptCancelButton.Location = new System.Drawing.Point(206, 118);
            this.OptCancelButton.Name = "OptCancelButton";
            this.OptCancelButton.Size = new System.Drawing.Size(71, 24);
            this.OptCancelButton.TabIndex = 5;
            this.OptCancelButton.Text = "Cancel";
            this.OptCancelButton.UseVisualStyleBackColor = true;
            this.OptCancelButton.Click += new System.EventHandler(this.CancelButton_Click);
            // 
            // OkButton
            // 
            this.OkButton.Location = new System.Drawing.Point(283, 118);
            this.OkButton.Name = "OkButton";
            this.OkButton.Size = new System.Drawing.Size(83, 24);
            this.OkButton.TabIndex = 6;
            this.OkButton.Text = "Ok";
            this.OkButton.UseVisualStyleBackColor = true;
            this.OkButton.Click += new System.EventHandler(this.OkButton_Click);
            // 
            // portBox
            // 
            this.portBox.Location = new System.Drawing.Point(67, 39);
            this.portBox.Maximum = new decimal(new int[] {
            65534,
            0,
            0,
            0});
            this.portBox.Name = "portBox";
            this.portBox.Size = new System.Drawing.Size(100, 20);
            this.portBox.TabIndex = 7;
            this.portBox.ValueChanged += new System.EventHandler(this.portBox_ValueChanged);
            // 
            // NetworkingBox
            // 
            this.NetworkingBox.Controls.Add(this.label1);
            this.NetworkingBox.Controls.Add(this.portBox);
            this.NetworkingBox.Controls.Add(this.label2);
            this.NetworkingBox.Controls.Add(this.hostnameBox);
            this.NetworkingBox.Controls.Add(this.CheckConnectionButton);
            this.NetworkingBox.Location = new System.Drawing.Point(12, 12);
            this.NetworkingBox.Name = "NetworkingBox";
            this.NetworkingBox.Size = new System.Drawing.Size(174, 100);
            this.NetworkingBox.TabIndex = 8;
            this.NetworkingBox.TabStop = false;
            this.NetworkingBox.Text = "Server";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.PortSelector);
            this.groupBox2.Controls.Add(this.BaudSelector);
            this.groupBox2.Controls.Add(this.CollarPortLabel);
            this.groupBox2.Controls.Add(this.CollarBaudLabel);
            this.groupBox2.Controls.Add(this.button3);
            this.groupBox2.Location = new System.Drawing.Point(192, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(174, 100);
            this.groupBox2.TabIndex = 9;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Collar";
            // 
            // PortSelector
            // 
            this.PortSelector.FormattingEnabled = true;
            this.PortSelector.Location = new System.Drawing.Point(67, 13);
            this.PortSelector.Name = "PortSelector";
            this.PortSelector.Size = new System.Drawing.Size(100, 21);
            this.PortSelector.TabIndex = 10;
            // 
            // BaudSelector
            // 
            this.BaudSelector.FormattingEnabled = true;
            this.BaudSelector.Location = new System.Drawing.Point(67, 38);
            this.BaudSelector.Name = "BaudSelector";
            this.BaudSelector.Size = new System.Drawing.Size(100, 21);
            this.BaudSelector.TabIndex = 10;
            // 
            // CollarPortLabel
            // 
            this.CollarPortLabel.AutoSize = true;
            this.CollarPortLabel.Location = new System.Drawing.Point(6, 16);
            this.CollarPortLabel.Name = "CollarPortLabel";
            this.CollarPortLabel.Size = new System.Drawing.Size(52, 13);
            this.CollarPortLabel.TabIndex = 0;
            this.CollarPortLabel.Text = "Portname";
            // 
            // CollarBaudLabel
            // 
            this.CollarBaudLabel.AutoSize = true;
            this.CollarBaudLabel.Location = new System.Drawing.Point(6, 42);
            this.CollarBaudLabel.Name = "CollarBaudLabel";
            this.CollarBaudLabel.Size = new System.Drawing.Size(50, 13);
            this.CollarBaudLabel.TabIndex = 1;
            this.CollarBaudLabel.Text = "Baudrate";
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(6, 65);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(160, 22);
            this.button3.TabIndex = 4;
            this.button3.Text = "Check Connection";
            this.button3.UseVisualStyleBackColor = true;
            // 
            // OptionsForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(30)))), ((int)(((byte)(30)))), ((int)(((byte)(30)))));
            this.ClientSize = new System.Drawing.Size(373, 152);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.NetworkingBox);
            this.Controls.Add(this.OkButton);
            this.Controls.Add(this.OptCancelButton);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "OptionsForm";
            this.Text = "Options";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.OnClosed);
            this.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OptionsForm_MouseDown);
            ((System.ComponentModel.ISupportInitialize)(this.portBox)).EndInit();
            this.NetworkingBox.ResumeLayout(false);
            this.NetworkingBox.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox hostnameBox;
		private System.Windows.Forms.Button CheckConnectionButton;
		private System.Windows.Forms.Button OptCancelButton;
		private System.Windows.Forms.Button OkButton;
		private System.Windows.Forms.NumericUpDown portBox;
		private System.Windows.Forms.GroupBox NetworkingBox;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Label CollarPortLabel;
		private System.Windows.Forms.Label CollarBaudLabel;
		private System.Windows.Forms.Button button3;
		private System.Windows.Forms.ComboBox PortSelector;
		private System.Windows.Forms.ComboBox BaudSelector;
	}
}